#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "cJSON.h"
#include "http_client.h"

#define TAG4 "HTTP CLIENT TASK"

extern EventGroupHandle_t task_formation_event_group;
#define ADC_TASK BIT3

EventGroupHandle_t http_client_event_group;
#define HTTP_NOTIFICATION_ALERT_REQUEST BIT0
#define HTTP_CREATE_ALERT_REQUEST BIT1

struct RequestParams requestparams;
HttpMethod method;
struct tm *timeinfo;

#define SUCCESS 200
#define FAIL 400
#define ERROR 500

char *buffer = NULL;
int indexPayloadBuffer = 0;

extern const uint8_t https_certificate_pem_start[] asm("_binary_https_certificate_pem_start");

esp_err_t clientEventHandler(esp_http_client_event_t *evt)
{
    struct RequestParams *requestparams = (struct RequestParams *)evt->user_data;
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGI(TAG4, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGI(TAG4, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGI(TAG4, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        //ESP_LOGI(TAG4, "HTTP_EVENT_ON_HEADER");
        //printf("%.*s\n", evt->data_len, (char*)evt->data);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGI(TAG4, "HTTP_EVENT_ON_DATA Len=%d", evt->data_len);
        printf("%.*s\n", evt->data_len, (char *)evt->data);
        if (buffer == NULL)
        {
            buffer = (char *)malloc(evt->data_len);
        }
        else
        {
            buffer = (char *)realloc(buffer, evt->data_len + indexPayloadBuffer);
        }
        memcpy(&buffer[indexPayloadBuffer], evt->data, evt->data_len);
        indexPayloadBuffer += evt->data_len;
        break;

    case HTTP_EVENT_ON_FINISH:
        ESP_LOGI(TAG4, "HTTP_EVENT_ON_FINISH");
        buffer = (char *)realloc(buffer, indexPayloadBuffer + 1);
        memcpy(&buffer[indexPayloadBuffer], "\0", 1);
        if (requestparams->OnGotData != NULL)
        {
            requestparams->OnGotData(buffer, requestparams->message);
        }
        free(buffer);
        buffer = NULL;
        indexPayloadBuffer = 0;
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(TAG4, "HTTP_EVENT_DISCONNECTED");
        break;
    }

    return ESP_OK;
}

void http_request(char *url)
{
    esp_http_client_config_t clientConfig = {
        .url = url,
        .event_handler = clientEventHandler,
        .user_data = &requestparams,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .cert_pem = (const char *)https_certificate_pem_start};

    esp_http_client_handle_t client = esp_http_client_init(&clientConfig);
    if (requestparams.method == Post)
    {
        esp_http_client_set_method(client, HTTP_METHOD_POST);
    }
    for (int i = 0; i < requestparams.headerCount; i++)
    {
        esp_http_client_set_header(client, requestparams.header[i].key, requestparams.header[i].val);
    }
    if(requestparams.body != NULL)
    {
        esp_http_client_set_post_field(client, requestparams.body, strlen(requestparams.body));
    }
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(TAG4, "Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
        requestparams.status = esp_http_client_get_status_code(client);
    }
    
    esp_http_client_cleanup(client);
}

void http_config_headers(HttpMethod method){
    requestparams.OnGotData = NULL;
    Header headerContentType = {
          .key = "Content-Type",
          .val = "application/json"};

    Header headerAccept = {
          .key = "Accept",
          .val = "application/json"};
    requestparams.header[0] = headerContentType;
    requestparams.header[1] = headerAccept;
    requestparams.headerCount = 2;
    requestparams.method = method;
}


void http_client(void *params){

    http_client_event_group = xEventGroupCreate();
    ESP_LOGI(TAG4,"-----------------------");
    ESP_LOGI(TAG4,"HTTP CLIENT INITIALIZED");
    ESP_LOGI(TAG4,"-----------------------");
    xEventGroupSetBits(task_formation_event_group, ADC_TASK);
    while(true){
        EventBits_t bits = xEventGroupWaitBits(http_client_event_group,
                                                HTTP_NOTIFICATION_ALERT_REQUEST | HTTP_CREATE_ALERT_REQUEST,
                                                pdFALSE,
                                                pdFALSE,
                                                portMAX_DELAY);

        if(bits & HTTP_NOTIFICATION_ALERT_REQUEST){
            ESP_LOGW(TAG4,"------------------ Send Notification -------------- ");
            xEventGroupClearBits(http_client_event_group,HTTP_NOTIFICATION_ALERT_REQUEST);
            time_t now = 0;
            time(&now);
            timeinfo = localtime(&now);
            char dateTimeBuffer[50];
            strftime(dateTimeBuffer, sizeof(dateTimeBuffer), "%c", timeinfo);
            ESP_LOGI(TAG4, "DateTime: %s\n", dateTimeBuffer);
            char *action = "publishAlert";
            char *type = "Sismica";
            char *message = "ALERTA SISMICA";
            char bodyBuffer[200];
            sprintf(bodyBuffer,
                    "{"
                    "  \"action\": \"%s\", "
                    "  \"type\": \"%s\", "
                    "  \"message\": \"%s\", "
                    "  \"dateTime\": \"%s\" "
                    "}",action,type,message,dateTimeBuffer);
            method=Post;
            http_config_headers(method);
            requestparams.body = bodyBuffer;
            http_request("https://4lczkvxvlf.execute-api.us-east-1.amazonaws.com/dev/controllerNotifications");
            if (requestparams.status == SUCCESS)
            {
                ESP_LOGI(TAG4, "Notication Sent");//<----- store successful alert to NVS
                vTaskDelay(pdMS_TO_TICKS(2000));
                xEventGroupSetBits(http_client_event_group, HTTP_CREATE_ALERT_REQUEST);
            }
            else
            {
                ESP_LOGE(TAG4, "Fail to send notification");//<------------------------store failed alert to NVS
            }
            
        }
        if(bits & HTTP_CREATE_ALERT_REQUEST){
            ESP_LOGW(TAG4,"------------------  Register Alert on DB -------------- ");
            xEventGroupClearBits(http_client_event_group,HTTP_CREATE_ALERT_REQUEST);
            char dateBuffer[10];
            char timeBuffer[10];
            char dayBuffer[10];
            strftime(dateBuffer, sizeof(dateBuffer), "%x", timeinfo);
            strftime(timeBuffer, sizeof(timeBuffer), "%X", timeinfo);
            strftime(dayBuffer, sizeof(dayBuffer), "%A", timeinfo);
            ESP_LOGI(TAG4, "Date: %s\t Time: %s\t Day: %s\n", dateBuffer,timeBuffer,dayBuffer);
            char *action = "createAlert";
            char *station = "Los santos";
            char *type = "Sismica";  
            char bodyBuffer[1024];
            sprintf(bodyBuffer,
                    "{"
                    "  \"action\": \"%s\", "
                    "  \"station\": \"%s\", "
                    "  \"type\": \"%s\", "
                    "  \"date\": \"%s\", "
                    "  \"time\": \"%s\", "
                    "  \"day\": \"%s\" "
                    "}",
                    action,station,type,dateBuffer,timeBuffer,dayBuffer);
            method=Post;  
            http_config_headers(method);  
            requestparams.body = bodyBuffer;  
            http_request("https://4lczkvxvlf.execute-api.us-east-1.amazonaws.com/dev/controllerAlerts");
            if (requestparams.status == SUCCESS)
            {
                ESP_LOGI(TAG4, "Entry Created");//<----- store successful alert to NVS
            }
            else
            {
                ESP_LOGE(TAG4, "Fail to create entry");//<------------------------store failed alert to NVS
            }
        }

    }
}