#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

char *TAG1 = "MAIN Task";

//--------CUSTOM MODULES-------------//
#include "wifi_connect.h"
#include "time_calibration.h"
#include "http_client.h"
#include "ADC.h"
//-------Event Group Tasks-----------//
EventGroupHandle_t task_formation_event_group;
#define WIFI_CONNECT_TASK BIT0
#define TIME_CALIBRATION_TASK BIT1
#define HTTP_CLIENT_TASK BIT2
#define ADC_TASK BIT3
void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    //---------------TEMP- RESET REGISTRY--------------
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    ESP_ERROR_CHECK(nvs_flash_init());
    nvs_handle handle;
    ESP_ERROR_CHECK(nvs_open("store", NVS_READWRITE, &handle));
    int32_t val = 0;
    esp_err_t result = nvs_get_i32(handle, "val", &val);
    switch (result)
    {
    case ESP_ERR_NVS_NOT_FOUND:
        ESP_LOGE(TAG1, "First Time!!");
        break;
    case ESP_OK:
        ESP_LOGI(TAG1, "Reset Number %d", val);
        break;
    default:
        ESP_LOGE(TAG1, "Error (%s) opening NVS handle!\n", esp_err_to_name(result));
        break;
    }
    val++;
    ESP_ERROR_CHECK(nvs_set_i32(handle, "val", val));
    ESP_ERROR_CHECK(nvs_commit(handle));
    nvs_close(handle);


    //-------------------------------------------------
    task_formation_event_group = xEventGroupCreate();
    ESP_LOGI(TAG1,"-----------------------");
    ESP_LOGI(TAG1,"MAIN Task initialized");
    ESP_LOGI(TAG1,"-----------------------");
    xEventGroupSetBits(task_formation_event_group, WIFI_CONNECT_TASK);
    while(true){
        EventBits_t bits = xEventGroupWaitBits(task_formation_event_group,
                                                WIFI_CONNECT_TASK | TIME_CALIBRATION_TASK | HTTP_CLIENT_TASK | ADC_TASK,
                                                pdFALSE,
                                                pdFALSE,
                                                portMAX_DELAY);
        if (bits & WIFI_CONNECT_TASK) {
            xEventGroupClearBits(task_formation_event_group,WIFI_CONNECT_TASK);
            xTaskCreatePinnedToCore(&wifi_connect, "wifi_connect_task", 1024 * 4, NULL, 1, NULL,0);
        } else if(bits & TIME_CALIBRATION_TASK){
            xEventGroupClearBits(task_formation_event_group,TIME_CALIBRATION_TASK);
            xTaskCreatePinnedToCore(&time_calibration, "time_calibration_task", 1024 * 2, NULL, 1, NULL,0);
        } else if(bits & HTTP_CLIENT_TASK){
            xEventGroupClearBits(task_formation_event_group,HTTP_CLIENT_TASK);
            xTaskCreatePinnedToCore(&http_client, "http_client_task", 1024 * 6, NULL, 1, NULL,0);
        }else if (bits & ADC_TASK) {
            xEventGroupClearBits(task_formation_event_group,ADC_TASK);
            xTaskCreatePinnedToCore(&adc_task, "ADC_TASK", 1024 * 2, NULL, 1, NULL,1);
            
         }
        else {
            ESP_LOGE(TAG1, "UNEXPECTED TASK");
        }
    }
    
}
