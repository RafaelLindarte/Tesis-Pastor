#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_sntp.h"

char *TAG3 = "TIME CALIBRATION TASK";

extern EventGroupHandle_t task_formation_event_group;
#define HTTP_CLIENT_TASK BIT2

EventGroupHandle_t time_event_group;
#define TIME_SYNC_STATUS BIT0
#define GET_SNTP_TIME BIT1


void getDateTime(char* dateTimeBuffer ){
    struct tm *timeinfo;
    time_t now = 0;
    time(&now);
    timeinfo = localtime(&now);
    char buffer[50];
    strftime(buffer, sizeof(buffer), "%c", timeinfo);
    strcpy(dateTimeBuffer, buffer);
}



void time_calibration_status(void){
    ESP_LOGW(TAG3,"Checking Time calibration Status");
    sntp_sync_status_t time_sync_status = sntp_get_sync_status();
    if(time_sync_status != SNTP_SYNC_STATUS_COMPLETED ){
        ESP_LOGW(TAG3,"Time not set yet");
        char buffer[50];
        getDateTime(&buffer[0]);
        ESP_LOGI(TAG3, "time: %s", buffer);
        xEventGroupSetBits(time_event_group, GET_SNTP_TIME);
    }else{
        sntp_stop(); 
        ESP_LOGW(TAG3,"Time Set");
        char buffer[50];
        getDateTime(&buffer[0]);
        ESP_LOGI(TAG3, "time: %s", buffer);
        xEventGroupSetBits(task_formation_event_group, HTTP_CLIENT_TASK);
    }
    
}

void set_time(struct timeval *tv)
{
    setenv("TZ", "GMT+5", 1);
    tzset();
    xEventGroupSetBits(time_event_group, TIME_SYNC_STATUS);
}

void get_sntp_time(void){
    
    sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
    sntp_setservername(0, "2.co.pool.ntp.org");
    sntp_init();
    sntp_set_time_sync_notification_cb(&set_time); 
}
void time_calibration (void *params){

    time_event_group = xEventGroupCreate();
    xEventGroupSetBits(time_event_group, TIME_SYNC_STATUS);
    ESP_LOGI(TAG3,"-----------------------");
    ESP_LOGI(TAG3,"TIME CALIBRATION INITIALIZED");
    ESP_LOGI(TAG3,"-----------------------");
    while(true){
        EventBits_t bits = xEventGroupWaitBits(time_event_group,
                                                TIME_SYNC_STATUS | GET_SNTP_TIME,
                                                pdFALSE,
                                                pdFALSE,
                                                portMAX_DELAY);

        if(bits & TIME_SYNC_STATUS){
            ESP_LOGW(TAG3,"------------------ Time Calibration Status -------------- ");
            time_calibration_status();
            xEventGroupClearBits(time_event_group,TIME_SYNC_STATUS);
        }
        if(bits & GET_SNTP_TIME){
            ESP_LOGW(TAG3,"------------------ Getting SNTP -------------- ");
            get_sntp_time();
            xEventGroupClearBits(time_event_group,GET_SNTP_TIME);
        }
    }
}