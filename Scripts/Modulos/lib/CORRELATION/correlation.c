#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include <math.h>
#include "driver/gpio.h"
char *TAG7 = "CORRELATION Task";

//-------Event Group Tasks-----------//
EventGroupHandle_t correlation_event_group;
#define CORR_COEFF BIT0
extern EventGroupHandle_t adc_event_group;
#define START_TIMER BIT0

// double coeff_X = 1, coeff_Y = 1, coeff_Z = 1;

// extern uint32_t num_samples;
// extern uint32_t *buffer_ch1;
// extern uint32_t *buffer_ch2;
// extern uint32_t *buffer_ch3;
// extern uint32_t *buffer_ch4;
// extern uint32_t *buffer_ch5;
// extern uint32_t *buffer_ch6;


//double n = 0;


// double correlationCoefficient6CH()
// {

//     // double corrTotal = 0.0;
// 	// double geo1_X = 0.0, geo1_Y = 0.0, geo1_Z = 0.0;
//     // double geo2_X = 0.0, geo2_Y = 0.0, geo2_Z = 0.0;
//     // double sum_X = 0.0, sum_Y = 0.0, sum_Z = 0.0;
// 	// double square_geo1_X = 0.0, square_geo1_Y = 0.0, square_geo1_Z = 0.0;
//     // double square_geo2_X = 0.0, square_geo2_Y = 0.0, square_geo2_Z = 0.0;
// 	// for (int i = 0; i < num_samples; i++)
// 	// {
// 	// 	// sum of elements of buffer_CH1.
//     //     geo1_X = geo1_X + buffer_ch1[i];
//     //     // sum of elements of buffer_CH2.
//     //     geo1_Y = geo1_Y + buffer_ch2[i];
//     //     // sum of elements of buffer_CH3.
//     //     geo1_Z = geo1_Z + buffer_ch3[i];
//     //     // sum of elements of buffer_CH4.
//     //     geo2_X = geo2_X + buffer_ch4[i];
//     //     // sum of elements of buffer_CH5.
//     //     geo2_Y = geo2_Y + buffer_ch5[i];
//     //     // sum of elements of buffer_CH6.
// 	// 	geo2_Z = geo2_Z + buffer_ch6[i];


// 	// 	// sum of X[i] * Y[i].
// 	// 	sum_X = sum_X + (buffer_ch1[i] * buffer_ch4[i]);
//     //     sum_Y = sum_Y + (buffer_ch2[i] * buffer_ch5[i]);
//     //     sum_Z = sum_Z + (buffer_ch3[i] * buffer_ch6[i]);
		
//     //     // sum of square of array elements.
//     //     square_geo1_X = square_geo1_X + (buffer_ch1[i] * buffer_ch1[i]);
//     //     square_geo1_Y = square_geo1_Y + (buffer_ch2[i] * buffer_ch2[i]);
//     //     square_geo1_Z = square_geo1_Z + (buffer_ch3[i] * buffer_ch3[i]);
//     //     square_geo2_X = square_geo2_X + (buffer_ch4[i] * buffer_ch4[i]);
//     //     square_geo2_Y = square_geo2_Y + (buffer_ch5[i] * buffer_ch5[i]);
//     //     square_geo2_Z = square_geo2_Z + (buffer_ch6[i] * buffer_ch6[i]);

// 	// }
    
// 	// use formula for calculating correlation coefficient.
// 	// double corr_X = ((num_samples * sum_X) - (geo1_X*geo2_X)) / sqrt((((num_samples * square_geo1_X) - (geo1_X * geo1_X)) * ((num_samples * square_geo2_X) - (geo2_X * geo2_X))));

//     // double corr_Y = ((num_samples * sum_Y) - (geo1_Y * geo2_Y)) / sqrt((((num_samples * square_geo1_Y) - (geo1_Y * geo1_Y)) * ((num_samples * square_geo2_Y) -( geo2_Y * geo2_Y))));

//     // double corr_Z = ((num_samples * sum_Z) - (geo1_Z * geo2_Z)) / sqrt((((num_samples * square_geo1_Z) - (geo1_Z * geo1_Z)) * ((num_samples * square_geo2_Z) -( geo2_Z * geo2_Z))));

    
// 	//return corrTotal = ((coeff_X * corr_X)+(coeff_Y * corr_Y)+(coeff_Z * corr_Z));
    
//     //return corr_Z = (!isnan(corr_Z)) ? corr_Z : 0;
//     return n++;
//     //return corr_Z;
// }
//----- Main Task -----//
void correlation_task(void *params)
{
    correlation_event_group = xEventGroupCreate();
    ESP_LOGI(TAG7,"-----------------------");
    ESP_LOGI(TAG7,"CORRELATION Task initialized");
    ESP_LOGI(TAG7,"-----------------------");
    xEventGroupSetBits(adc_event_group, START_TIMER);
    while(true){
        EventBits_t bits = xEventGroupWaitBits(correlation_event_group,
                                                CORR_COEFF,
                                                pdFALSE,
                                                pdFALSE,
                                                portMAX_DELAY);

        if (bits & CORR_COEFF) {
            xEventGroupClearBits(correlation_event_group,CORR_COEFF);
            //double r = correlationCoefficient6CH();
            
            //printf("corr: %f\n",r);
            xEventGroupSetBits(adc_event_group, START_TIMER);
            
        }
        else {
            ESP_LOGE(TAG7, "UNEXPECTED TASK");
        }
    }
    
}
