#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

char *TAG2 = "WIFI CONNECT TASK";

char Network_ssid[32];
char Network_pwd[64];

const char * hostname = "pastorV3-station.local.net";
extern EventGroupHandle_t task_formation_event_group;
#define TIME_CALIBRATION_TASK BIT1
EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

EventGroupHandle_t connect_event_group;
#define WIFI_STATUS BIT0
#define WIFI_RECONNECT BIT1
#define WIFI_SCAN BIT2
#define WIFI_CONNECT_TO_NETWORK BIT3


static int s_retry_num = 0;
//---------------Network Interface---------//
esp_netif_t *sta_netif;
esp_event_handler_instance_t instance_any_id;
esp_event_handler_instance_t instance_got_ip;

//--------------Function Prototipes-----------//
void wifi_init_netif(void);
void wifi_connect_sta(void);
void wifi_scan(void);
void wifi_close_sta(void);
void wifi_reconnect(void);

//-------------------Wifi Event Handler ----------------------//
static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < 5) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG2, "retry to connect to the AP");
        } else {
            ESP_LOGI(TAG2,"connect to the AP fail");
            xEventGroupSetBits(connect_event_group, WIFI_STATUS);
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG2, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(connect_event_group, WIFI_STATUS);
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}
//--------------------------Wifi Custom Functions--------------//
void wifi_init_netif(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    sta_netif = esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    //---HOST NAME------
    esp_err_t err;
    if( (err = tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA,hostname)) != ESP_OK){
        fprintf(stderr,"Error setting Esp hostname: %s",esp_err_to_name(err));
    };
    //------------------
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
                                                        
    //ESP_LOGW(TAG2, "==============> wifi_init_netif finished.");
}

void wifi_connect_sta(void){
    wifi_config_t wifi_config = {
        .sta = {
	        .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    strcpy((char*)wifi_config.sta.ssid,Network_ssid);
    strcpy((char*)wifi_config.sta.password,Network_pwd);
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR));


    esp_err_t error = esp_wifi_connect();
    if(error != ESP_OK){
        ESP_LOGW(TAG2, "Provided Networks are not avaliable.");
        wifi_close_sta();
    }
    else{
        ESP_LOGW(TAG2, "Successful connection to the provided networks");
    }
    

}
void wifi_scan(void){

    uint16_t number = CONFIG_NETWORK_SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[CONFIG_NETWORK_SCAN_LIST_SIZE];
    uint16_t ap_count = 0;
    int32_t prev_rssi = -100;
    memset(ap_info, 0, sizeof(ap_info));
    ESP_LOGI(TAG2,"Scaning..");
    ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    ESP_LOGI(TAG2,"--------------Avaliable Networks--------------");
    ESP_LOGI(TAG2, "Total APs scanned ===> %u", ap_count);
    for (int i = 0; (i < CONFIG_NETWORK_SCAN_LIST_SIZE) && (i < ap_count); i++) {
        if(strcmp((char*)ap_info[i].ssid,(char*)CONFIG_PRIMARY_SSID) == 0 || strcmp((char*)ap_info[i].ssid,(char*)CONFIG_SECONDARY_SSID) == 0){
            ESP_LOGW(TAG2, "SSID \t\t%s\n", ap_info[i].ssid);
            if((int)ap_info[i].rssi > prev_rssi){
                prev_rssi = (int)ap_info[i].rssi;
                ESP_LOGW(TAG2,"Best Network: %s\n",ap_info[i].ssid);
                if(strcmp((char*)ap_info[i].ssid,(char*)CONFIG_PRIMARY_SSID) == 0){
                    strcpy(Network_ssid,(char*)CONFIG_PRIMARY_SSID);
                    strcpy(Network_pwd,(char*)CONFIG_PRIMARY_PASSWORD);
                }
                else{
                    strcpy(Network_ssid,(char*)CONFIG_SECONDARY_SSID);
                    strcpy(Network_pwd,(char*)CONFIG_SECONDARY_PASSWORD);
                }
            }
        }
        else{
            ESP_LOGI(TAG2, "SSID \t\t%s\n", ap_info[i].ssid);
        }
    }
    //ESP_LOGW(TAG2, "==============> wifi_scan finished.");
}

void wifi_close_sta(void){
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    ESP_ERROR_CHECK(esp_event_loop_delete_default());
    ESP_ERROR_CHECK(esp_wifi_restore());
    esp_netif_destroy(sta_netif);
    //ESP_LOGW(TAG2, "==============> wifi_close_sta finished.");
}

void wifi_status_handler(void){

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                                WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                                pdFALSE,
                                                pdFALSE,
                                                portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        xEventGroupClearBits(s_wifi_event_group,WIFI_CONNECTED_BIT);
        ESP_LOGI(TAG2, "connected to ap SSID:%s password:%s",Network_ssid, Network_pwd);       
        xEventGroupSetBits(task_formation_event_group, TIME_CALIBRATION_TASK);
    } else if (bits & WIFI_FAIL_BIT) {
        xEventGroupClearBits(s_wifi_event_group,WIFI_CONNECTED_BIT);
        ESP_LOGI(TAG2, "Failed to connect to SSID:%s, password:%s",
                Network_ssid, Network_pwd);
        s_retry_num = 0;
        xEventGroupSetBits(connect_event_group, WIFI_RECONNECT);
    } else {
        ESP_LOGE(TAG2, "UNEXPECTED EVENT");
    }
}

void wifi_reconnect(void){
    wifi_scan();
    wifi_connect_sta();
}

//-----------MAIN TASK -----------//
void wifi_connect( void *params ){

    wifi_init_netif();
    wifi_scan();
    wifi_connect_sta();
    connect_event_group = xEventGroupCreate();
    ESP_LOGI(TAG2,"-----------------------");
    ESP_LOGI(TAG2,"WIFI CONNECT INITIALIZED");
    ESP_LOGI(TAG2,"-----------------------");
    while(true){
        EventBits_t bits = xEventGroupWaitBits(connect_event_group,
                                                WIFI_STATUS | WIFI_RECONNECT | WIFI_SCAN | WIFI_CONNECT_TO_NETWORK,
                                                pdFALSE,
                                                pdFALSE,
                                                portMAX_DELAY);

        if(bits & WIFI_STATUS){
            ESP_LOGW(TAG2,"------------------ Wifi Status -------------- ");
            wifi_status_handler();
            xEventGroupClearBits(connect_event_group,WIFI_STATUS);
        }
        if(bits & WIFI_RECONNECT){
            ESP_LOGW(TAG2,"------------------ Wifi Reconnecting -------------- ");
            wifi_reconnect();
            xEventGroupClearBits(connect_event_group,WIFI_RECONNECT);
        }
        if(bits & WIFI_SCAN){
            ESP_LOGW(TAG2,"------------------ Wifi Scan -------------- ");
            wifi_scan();
            xEventGroupClearBits(connect_event_group,WIFI_RECONNECT);
        }
        if(bits & WIFI_CONNECT_TO_NETWORK){
            ESP_LOGW(TAG2,"------------------ Wifi Conecting to Selected Network -------------- ");
            wifi_reconnect();
            xEventGroupClearBits(connect_event_group,WIFI_RECONNECT);
        }
    }
}