#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "time_calibration.h"
#include "internal_storage.h"

#define TAG5 "INTERNAL STORAGE TASK"

EventGroupHandle_t internal_storage_event_group;
#define LIST_EVENTS BIT0
#define INTERRRUPT_EVENT BIT1
#define HTTP_EVENT BIT2

extern EventGroupHandle_t task_formation_event_group;
#define ADC_TASK BIT4


void listEvents(){}
void storeEvent(){}


void internal_storage(void *params){
    internal_storage_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(nvs_flash_init_partition("internal_storage"));
    ESP_LOGI(TAG5,"-----------------------");
    ESP_LOGI(TAG5,"INTERNAL STORAGE INITIALIZED");
    ESP_LOGI(TAG5,"-----------------------");
    //xEventGroupSetBits(task_formation_event_group, ADC_TASK);
    while(true){
        EventBits_t bits = xEventGroupWaitBits(internal_storage_event_group,
                                                LIST_EVENTS | INTERRRUPT_EVENT | HTTP_EVENT,
                                                pdFALSE,
                                                pdFALSE,
                                                portMAX_DELAY);

        if(bits & LIST_EVENTS){
            ESP_LOGW(TAG5,"------------------ List Stored Events -------------- ");
            xEventGroupClearBits(internal_storage_event_group,LIST_EVENTS);
            // nvs_handle handle;
            // ESP_ERROR_CHECK(nvs_open_from_partition("internal_storage", "interruptEvent", NVS_READWRITE, &handle));
            
            nvs_iterator_t it1 = nvs_entry_find("internal_storage", "interruptEvent", NVS_TYPE_BLOB);
            nvs_entry_info_t info;
            while (it1 != NULL) {
                nvs_entry_info(it1, &info);
                it1 = nvs_entry_next(it1);
                printf("nameSpace: '%s',key: '%s', type: '%d' \n", info.namespace_name, info.key, info.type);
            };
            
            //nvs_close(handle);
        }
        if(bits & INTERRRUPT_EVENT){
            ESP_LOGW(TAG5,"------------------ Store Interrupt Event -------------- ");
            xEventGroupClearBits(internal_storage_event_group,INTERRRUPT_EVENT);
            nvs_handle handle;
            ESP_ERROR_CHECK(nvs_open_from_partition("internal_storage", "interruptEvent", NVS_READWRITE, &handle));
            
            nvs_iterator_t it = nvs_entry_find("internal_storage", "interruptEvent", NVS_TYPE_BLOB);
            nvs_entry_info_t info;
            while (it != NULL) {
                nvs_entry_info(it, &info);
                it = nvs_entry_next(it);
            };
            //printf("Latest Entry ==> nameSpace: '%s',key: '%s', type: '%d' \n", info.namespace_name, info.key, info.type);
            char event_key[20];
            if(strcmp(info.key,"")==0){
                
                sprintf(event_key, "event_%d", 0);
                nvs_event event;
                event.id = 0;
                sprintf(event.type,"interrupt");
                sprintf(event.data,"interrupt event detected");
                getDateTime(&event.dateTimeBuffer[0]);
                ESP_ERROR_CHECK(nvs_set_blob(handle, event_key, (void *) &event, sizeof(event) ));
                ESP_ERROR_CHECK(nvs_commit(handle));
            }else{
                
                sprintf(event_key, "event_%c", (char)((int)info.key[strlen(info.key)-1])+1);
                nvs_event event;
                event.id = 0;
                sprintf(event.type,"interrupt");
                sprintf(event.data,"interrupt event detected");
                getDateTime(&event.dateTimeBuffer[0]);
                ESP_ERROR_CHECK(nvs_set_blob(handle, event_key, (void *) &event, sizeof(event) ));
                ESP_ERROR_CHECK(nvs_commit(handle));
                
            }
            nvs_close(handle);
            xEventGroupSetBits(internal_storage_event_group, LIST_EVENTS);
        }
        if(bits & HTTP_EVENT){
            ESP_LOGW(TAG5,"------------------ Store Http Event -------------- ");
            xEventGroupClearBits(internal_storage_event_group,HTTP_EVENT);
            nvs_handle handle;
            ESP_ERROR_CHECK(nvs_open_from_partition("internal_storage", "httpEvent", NVS_READWRITE, &handle));
            nvs_iterator_t it = nvs_entry_find("internal_storage", "httpEvent", NVS_TYPE_BLOB);
            nvs_entry_info_t info;
            while (it != NULL) {
                nvs_entry_info(it, &info);
                it = nvs_entry_next(it);
            };
            //printf("Latest Entry ==> nameSpace: '%s',key: '%s', type: '%d' \n", info.namespace_name, info.key, info.type);
            char event_key[20];
            if(strcmp(info.key,"")==0){
                
                sprintf(event_key, "event_%d", 0);
                nvs_event event;
                event.id = 0;
                sprintf(event.type,"http");
                sprintf(event.data,"http event detected");
                getDateTime(&event.dateTimeBuffer[0]);
                ESP_ERROR_CHECK(nvs_set_blob(handle, event_key, (void *) &event, sizeof(event) ));
                ESP_ERROR_CHECK(nvs_commit(handle));
            }else{
                
                sprintf(event_key, "event_%c", (char)((int)info.key[strlen(info.key)-1])+1);
                nvs_event event;
                event.id = 0;
                sprintf(event.type,"http");
                sprintf(event.data,"http event detected");
                getDateTime(&event.dateTimeBuffer[0]);
                ESP_ERROR_CHECK(nvs_set_blob(handle, event_key, (void *) &event, sizeof(event) ));
                ESP_ERROR_CHECK(nvs_commit(handle));
            }
            nvs_close(handle);
        }

    }
}