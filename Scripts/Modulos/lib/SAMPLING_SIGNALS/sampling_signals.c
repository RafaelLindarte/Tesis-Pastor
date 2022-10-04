#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_log.h"
#include "esp_types.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "driver/gpio.h"

char *TAG7 = "SAMPLIG SIGNALS TASK";

#define PIN 23
int isOn = 0;
EventGroupHandle_t sampling_signals_event_group;
#define START_TIMER BIT0
#define SAMPLE_SIGNAL BIT1
#define STOP_TIMER BIT2
extern EventGroupHandle_t event_detection_event_group;
#define CORR_COEFF BIT0
extern EventGroupHandle_t task_formation_event_group;
#define EVENT_DETECTION_TASK BIT6

#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate

#define TIMER_DIVIDER         16  //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // convert counter value to seconds
#define TIMER_INTERVAL_SEC   (0.001)   // sample test interval for the timer

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel1 = ADC1_CHANNEL_0;/*!< ADC1 channel 0 is GPIO36 */
static const adc_channel_t channel2 = ADC1_CHANNEL_3;/*!< ADC1 channel 3 is GPIO39 */
static const adc_channel_t channel3 = ADC1_CHANNEL_6;/*!< ADC1 channel 6 is GPIO34 */
static const adc_channel_t channel4 = ADC1_CHANNEL_7;/*!< ADC1 channel 7 is GPIO35 */
static const adc_channel_t channel5 = ADC1_CHANNEL_4;/*!< ADC1 channel 4 is GPIO32 */
static const adc_channel_t channel6 = ADC1_CHANNEL_5;/*!< ADC1 channel 5 is GPIO33 */
    
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;
static const adc_atten_t atten = ADC_ATTEN_DB_11;

uint32_t num_samples = 250;
uint32_t *buffer_ch1 = NULL;
uint32_t *buffer_ch2 = NULL;
uint32_t *buffer_ch3 = NULL;
uint32_t *buffer_ch4 = NULL;
uint32_t *buffer_ch5 = NULL;
uint32_t *buffer_ch6 = NULL;
int indexBuffer = 0;


//----- internal functions -----//
static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    } else {
        printf("Characterized using Default Vref\n");
    }
}

static void check_efuse(void)
{
    //Check if TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("eFuse Two Point: NOT supported\n");
    }
    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        printf("eFuse Vref: Supported\n");
    } else {
        printf("eFuse Vref: NOT supported\n");
    }
}

void sampling_signal(){
    
    //uint32_t ch1_reading = 0,ch2_reading = 0,ch3_reading = 0,ch4_reading = 0,ch5_reading = 0,ch6_reading = 0;
    
    buffer_ch1[indexBuffer] = adc1_get_raw((adc1_channel_t)channel1);
    buffer_ch2[indexBuffer] = adc1_get_raw((adc1_channel_t)channel2); 
    buffer_ch3[indexBuffer] = adc1_get_raw((adc1_channel_t)channel3); 
    buffer_ch4[indexBuffer] = adc1_get_raw((adc1_channel_t)channel4); 
    buffer_ch5[indexBuffer] = adc1_get_raw((adc1_channel_t)channel5); 
    buffer_ch6[indexBuffer] = adc1_get_raw((adc1_channel_t)channel6);
    // //Convert adc_reading to voltage in mV
    // buffer_ch1[indexBuffer] = esp_adc_cal_raw_to_voltage(ch1_reading, adc_chars);
    // buffer_ch2[indexBuffer] = esp_adc_cal_raw_to_voltage(ch2_reading, adc_chars);
    // buffer_ch3[indexBuffer] = esp_adc_cal_raw_to_voltage(ch3_reading, adc_chars);
    // buffer_ch4[indexBuffer] = esp_adc_cal_raw_to_voltage(ch4_reading, adc_chars);
    // buffer_ch5[indexBuffer] = esp_adc_cal_raw_to_voltage(ch5_reading, adc_chars);
    // buffer_ch6[indexBuffer] = esp_adc_cal_raw_to_voltage(ch6_reading, adc_chars); 
    //printf("%d,%d,%d,%d,%d,%d\r",buffer_ch1[indexBuffer],buffer_ch2[indexBuffer],buffer_ch3[indexBuffer],buffer_ch4[indexBuffer],buffer_ch5[indexBuffer],buffer_ch6[indexBuffer]);
    indexBuffer++;
}

void IRAM_ATTR timer_group0_isr(void *para)
{
    timer_spinlock_take(TIMER_GROUP_0);
    
    int timer_idx = (int) para;

    uint32_t timer_intr = timer_group_get_intr_status_in_isr(TIMER_GROUP_0);
       
    if (timer_intr & TIMER_INTR_T1) {
        timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, TIMER_1);
    }
    timer_group_enable_alarm_in_isr(TIMER_GROUP_0, timer_idx);
    BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdTRUE;
    xEventGroupSetBitsFromISR(sampling_signals_event_group, SAMPLE_SIGNAL,&xHigherPriorityTaskWoken);
    if(indexBuffer>=num_samples)xEventGroupSetBitsFromISR(sampling_signals_event_group, STOP_TIMER,pdFALSE);
    
    timer_spinlock_give(TIMER_GROUP_0);
}
static void start_timer(int timer_idx,bool auto_reload, double timer_interval_sec)
{

    /* Select and initialize basic parameters of the timer */
    timer_config_t config = {
        .divider = TIMER_DIVIDER,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .auto_reload = 1,
    }; // default clock source is APB
    timer_init(TIMER_GROUP_0, TIMER_1, &config);
    /* Timer's counter will initially start from value below.
       Also, if auto_reload is set, this value will be automatically reload on alarm */
    timer_set_counter_value(TIMER_GROUP_0, timer_idx, 0x00000000ULL);

    /* Configure the alarm value and the interrupt on alarm. */
    timer_set_alarm_value(TIMER_GROUP_0, timer_idx, timer_interval_sec * TIMER_SCALE);
    timer_enable_intr(TIMER_GROUP_0, timer_idx);
    timer_isr_register(TIMER_GROUP_0, timer_idx, timer_group0_isr,
                       (void *)timer_idx, ESP_INTR_FLAG_IRAM, NULL);

    timer_start(TIMER_GROUP_0, timer_idx);
}
//----- external functions -----//
//----- Main Task -----//
void sampling_signals_task(void *params){
    //Configure ADC
    adc1_config_width(width);
    adc1_config_channel_atten(channel1, atten);
    adc1_config_channel_atten(channel2, atten);
    adc1_config_channel_atten(channel3, atten);
    adc1_config_channel_atten(channel4, atten);
    adc1_config_channel_atten(channel5, atten);
    adc1_config_channel_atten(channel6, atten);
    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, atten, width, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);
    //Initialize ADC buffer
    
    if (buffer_ch1 == NULL && buffer_ch2 == NULL && buffer_ch3 == NULL && buffer_ch4 == NULL && buffer_ch5 == NULL && buffer_ch6 == NULL)
    {
        buffer_ch1 = (uint32_t *)malloc(num_samples*sizeof(uint32_t));
        buffer_ch2 = (uint32_t *)malloc(num_samples*sizeof(uint32_t));
        buffer_ch3 = (uint32_t *)malloc(num_samples*sizeof(uint32_t));
        buffer_ch4 = (uint32_t *)malloc(num_samples*sizeof(uint32_t));
        buffer_ch5 = (uint32_t *)malloc(num_samples*sizeof(uint32_t));
        buffer_ch6 = (uint32_t *)malloc(num_samples*sizeof(uint32_t));
    }
    
    //gpio_pad_select_gpio(PIN);
    //gpio_set_direction(PIN, GPIO_MODE_OUTPUT);
    //gpio_set_level(PIN, 0);

    sampling_signals_event_group = xEventGroupCreate();
    ESP_LOGI(TAG7,"-----------------------");
    ESP_LOGI(TAG7,"SAMPLIG SIGNAL INITIALIZED");
    ESP_LOGI(TAG7,"-----------------------");
    xEventGroupSetBits(task_formation_event_group, EVENT_DETECTION_TASK);
    
    while (true)
    {
        EventBits_t bits = xEventGroupWaitBits(sampling_signals_event_group,
                                                START_TIMER | SAMPLE_SIGNAL | STOP_TIMER,
                                                pdFALSE,
                                                pdFALSE,
                                                portMAX_DELAY);

        if (bits & START_TIMER) {
            //----- Clear flag
            xEventGroupClearBits(sampling_signals_event_group,START_TIMER);
            //----- Code
            if(indexBuffer == 0){
                start_timer(TIMER_1, 1, TIMER_INTERVAL_SEC);
            }
            //printf("Timer Running\n");
        }else if(bits & SAMPLE_SIGNAL){
            //----- Clear flag
            xEventGroupClearBits(sampling_signals_event_group,SAMPLE_SIGNAL);
            //----- Code
            //printf("Sampling Signals\n");
            //gpio_set_level(PIN, 1);
            sampling_signal();
            //gpio_set_level(PIN, 0);
            
        }
        else if(bits & STOP_TIMER){
            //----- Clear flag
            xEventGroupClearBits(sampling_signals_event_group,STOP_TIMER);
            //----- Code
            timer_pause(TIMER_GROUP_0, TIMER_1);
            //printf("Buffers Done\n");
            indexBuffer = 0;
            //xEventGroupSetBits(sampling_signals_event_group, START_TIMER);
            xEventGroupSetBits(event_detection_event_group, CORR_COEFF);
        }
        else {
            ESP_LOGE(TAG7, "UNEXPECTED TASK");
        }
    } 
    
}