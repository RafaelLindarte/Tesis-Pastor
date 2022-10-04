#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_heap_caps.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_task_wdt.h"

#include "samples.h"
double corr_coeff_X = 1.0, corr_coeff_Y = 1.0, corr_coeff_Z = 1.0;
double rms_coeff_X = 1.0, rms_coeff_Y = 1.0, rms_coeff_Z = 1.0;

void event_detection(int *buffer1,int *buffer2,int *buffer3,int *buffer4,int *buffer5,int *buffer6){
	int32_t sum_geo1_X = 0,sum_geo1_Y = 0,sum_geo1_Z = 0,sum_geo2_X = 0,sum_geo2_Y = 0,sum_geo2_Z = 0;
	int64_t sum_XX = 0,sum_YY = 0,sum_ZZ = 0;
	uint64_t sum_square_geo1_X = 0,sum_square_geo1_Y = 0,sum_square_geo1_Z = 0,sum_square_geo2_X = 0,sum_square_geo2_Y = 0,sum_square_geo2_Z = 0;

	for (int i = 0; i < lta_size; i++)
	{

		// sum of elements of buffer_CH1.
		sum_geo1_X = sum_geo1_X + buffer1[i];
		// sum of elements of buffer_CH2.
		sum_geo1_Y = sum_geo1_Y + buffer2[i];
		// sum of elements of buffer_CH3.
		sum_geo1_Z = sum_geo1_Z + buffer3[i];
		// sum of elements of buffer_CH4.
		sum_geo2_X = sum_geo2_X + buffer4[i];
		// sum of elements of buffer_CH5.
		sum_geo2_Y = sum_geo2_Y + buffer5[i];
		// sum of elements of buffer_CH6.
		sum_geo2_Z = sum_geo2_Z + buffer6[i];


		// sum of X[i] * Y[i].
		sum_XX = sum_XX + (buffer1[i] * buffer4[i]);
		sum_YY = sum_YY + (buffer2[i] * buffer5[i]);
		sum_ZZ = sum_ZZ + (buffer3[i] * buffer6[i]);

		// sum of square of array elements.
		sum_square_geo1_X = sum_square_geo1_X + (buffer1[i] * buffer1[i]);
		sum_square_geo1_Y = sum_square_geo1_Y + (buffer2[i] * buffer2[i]);
		sum_square_geo1_Z = sum_square_geo1_Z + (buffer3[i] * buffer3[i]);
		sum_square_geo2_X = sum_square_geo2_X + (buffer4[i] * buffer4[i]);
		sum_square_geo2_Y = sum_square_geo2_Y + (buffer5[i] * buffer5[i]);
		sum_square_geo2_Z = sum_square_geo2_Z + (buffer6[i] * buffer6[i]);

	}
////	printf("sum_geo1_X: %ld,sum_geo1_Y: %ld,sum_geo1_Z: %ld,sum_geo2_X: %ld,sum_geo2_Y: %ld, sum_geo2_Z: %ld\n",sum_geo1_X,sum_geo1_Y,sum_geo1_Z,sum_geo2_X,sum_geo2_Y,sum_geo2_Z);
////	printf("sum_square_geo1_X: %lld,sum_square_geo1_Y: %lld,sum_square_geo1_Z: %lld,sum_square_geo2_X: %lld,sum_square_geo2_Y: %lld,sum_square_geo2_Z: %lld\n",sum_square_geo1_X,sum_square_geo1_Y,sum_square_geo1_Z,sum_square_geo2_X,sum_square_geo2_Y,sum_square_geo2_Z);
////	printf("sum_XX: %lld,sum_YY: %lld,sum_ZZ: %lld\n",sum_XX,sum_YY,sum_ZZ);
////
////	printf("num %lld\n",((sum_XX) - ((sum_geo1_X * sum_geo2_X ) / num_samples)));
////	printf("dem %f\n",sqrt(((sum_square_geo1_X) - ( (sum_geo1_X * sum_geo1_X) /num_samples )) * ((sum_square_geo2_X) - ( (sum_geo2_X * sum_geo2_X) / num_samples )) ));
//	double corr_X = ((sum_XX) - ((sum_geo1_X * sum_geo2_X) / num_samples)) / sqrt(((sum_square_geo1_X) - ( (sum_geo1_X * sum_geo1_X) / num_samples)) * ((sum_square_geo2_X) - ( (sum_geo2_X * sum_geo2_X) / num_samples )));
//	double corr_Y = ((sum_YY) - ((sum_geo1_Y * sum_geo2_Y) / num_samples)) / sqrt(((sum_square_geo1_Y) - ( (sum_geo1_Y * sum_geo1_Y) / num_samples)) * ((sum_square_geo2_Y) - ( (sum_geo2_Y * sum_geo2_Y) / num_samples )));
//	double corr_Z = ((sum_ZZ) - ((sum_geo1_Z * sum_geo2_Z) / num_samples)) / sqrt(((sum_square_geo1_Z) - ( (sum_geo1_Z * sum_geo1_Z) / num_samples)) * ((sum_square_geo2_Z) - ( (sum_geo2_Z * sum_geo2_Z) / num_samples )));
//
//	if( !isnan(corr_X) || !isinf(corr_X) ){
//		corr_X = corr_X < 0 ? corr_X * (-1): corr_X;
//	}
//	if( !isnan(corr_Y) || !isinf(corr_Y) ){
//		corr_Y = corr_Y < 0 ? corr_Y * (-1): corr_Y;
//	}
//	if( !isnan(corr_Z) || !isinf(corr_Z) ){
//		corr_Z = corr_Z < 0 ? corr_Z * (-1): corr_Z;
//	}
//	printf("corrX: %f,corrY: %f,corrZ: %f\n",corr_X,corr_Y,corr_Z);
//	if((corr_X >= corr_coeff_X) && (corr_Y >= corr_coeff_Y) && (corr_Z >= corr_coeff_Z)){
//		printf("ALERTA HP!!\n");
//	}

	double lta_prom_geo1_X = sum_geo1_X / lta_size < 0 ? sum_geo1_X / lta_size * (-1): sum_geo1_X / lta_size;
	double lta_prom_geo1_Y = sum_geo1_Y / lta_size < 0 ? sum_geo1_Y / lta_size * (-1): sum_geo1_Y / lta_size;
	double lta_prom_geo1_Z = sum_geo1_Z / lta_size < 0 ? sum_geo1_Z / lta_size * (-1): sum_geo1_Z / lta_size;
	double lta_prom_geo2_X = sum_geo2_X / lta_size < 0 ? sum_geo2_X / lta_size * (-1): sum_geo2_X / lta_size;
	double lta_prom_geo2_Y = sum_geo2_Y / lta_size < 0 ? sum_geo2_Y / lta_size * (-1): sum_geo2_Y / lta_size;
	double lta_prom_geo2_Z = sum_geo2_Z / lta_size < 0 ? sum_geo2_Z / lta_size * (-1): sum_geo2_Z / lta_size;


	printf("lta_prom_X:%f\n",lta_prom_geo1_X);
	printf("lta_prom_Y:%f\n",lta_prom_geo1_Y);
	printf("lta_prom_Z:%f\n",lta_prom_geo1_Z);
//
//	printf("lta_prom_X:%f\n",lta_prom_geo2_X);
//	printf("lta_prom_Y:%f\n",lta_prom_geo2_Y);
//	printf("lta_prom_Z:%f\n",lta_prom_geo2_Z);

	int16_t *sta_buffer_geo1_X = (int16_t*)malloc(sta_size*sizeof(int16_t));
	int16_t *sta_buffer_geo1_Y = (int16_t*)malloc(sta_size*sizeof(int16_t));
	int16_t *sta_buffer_geo1_Z = (int16_t*)malloc(sta_size*sizeof(int16_t));
	int16_t *sta_buffer_geo2_X = (int16_t*)malloc(sta_size*sizeof(int16_t));
	int16_t *sta_buffer_geo2_Y = (int16_t*)malloc(sta_size*sizeof(int16_t));
	int16_t *sta_buffer_geo2_Z = (int16_t*)malloc(sta_size*sizeof(int16_t));

	int32_t sta_sum_geo1_X = 0,sta_sum_geo1_Y = 0,sta_sum_geo1_Z = 0,sta_sum_geo2_X = 0,sta_sum_geo2_Y = 0,sta_sum_geo2_Z = 0;
	int i=0;
	while(i < lta_size){
		memcpy(&sta_buffer_geo1_X[0],&buffer1[i],sta_size*sizeof(int16_t));
		memcpy(&sta_buffer_geo1_Y[0],&buffer2[i],sta_size*sizeof(int16_t));
		memcpy(&sta_buffer_geo1_Z[0],&buffer3[i],sta_size*sizeof(int16_t));
		memcpy(&sta_buffer_geo2_X[0],&buffer4[i],sta_size*sizeof(int16_t));
		memcpy(&sta_buffer_geo2_Y[0],&buffer5[i],sta_size*sizeof(int16_t));
		memcpy(&sta_buffer_geo2_Z[0],&buffer6[i],sta_size*sizeof(int16_t));
		for(int j = 0; j < sta_size; j++){
			sta_sum_geo1_X = sta_sum_geo1_X + sta_buffer_geo1_X[j];
			sta_sum_geo1_Y = sta_sum_geo1_Y + sta_buffer_geo1_Y[j];
			sta_sum_geo1_Z = sta_sum_geo1_Z + sta_buffer_geo1_Z[j];
			sta_sum_geo2_X = sta_sum_geo2_X + sta_buffer_geo2_X[j];
			sta_sum_geo2_Y = sta_sum_geo2_Y + sta_buffer_geo2_Y[j];
			sta_sum_geo2_Z = sta_sum_geo2_Z + sta_buffer_geo2_Z[j];
		}

		double sta_prom_geo1_X = sta_sum_geo1_X / sta_size < 0 ? sta_sum_geo1_X / sta_size * (-1): sta_sum_geo1_X / sta_size;
		double sta_prom_geo1_Y = sta_sum_geo1_Y / sta_size < 0 ? sta_sum_geo1_Y / sta_size * (-1): sta_sum_geo1_Y / sta_size;
		double sta_prom_geo1_Z = sta_sum_geo1_Z / sta_size < 0 ? sta_sum_geo1_Z / sta_size * (-1): sta_sum_geo1_Z / sta_size;
		double sta_prom_geo2_X = sta_sum_geo2_X / sta_size < 0 ? sta_sum_geo2_X / sta_size * (-1): sta_sum_geo2_X / sta_size;
		double sta_prom_geo2_Y = sta_sum_geo2_Y / sta_size < 0 ? sta_sum_geo2_Y / sta_size * (-1): sta_sum_geo2_Y / sta_size;
		double sta_prom_geo2_Z = sta_sum_geo2_Z / sta_size < 0 ? sta_sum_geo2_Z / sta_size * (-1): sta_sum_geo2_Z / sta_size;

//		printf("sta_prom_X:%2.f        Delta: %2.f \t sta_prom_Y:%2.f        Delta: %2.f \t sta_prom_Z:%2.f        Delta:%2.f\n",
//				sta_prom_geo1_X,sta_prom_geo1_X - lta_prom_geo1_X,
//				sta_prom_geo1_Y,sta_prom_geo1_Y - lta_prom_geo1_Y,
//				sta_prom_geo1_Z,sta_prom_geo1_Z - lta_prom_geo1_Z);
		printf("STA/LTA X:%f,Y:%f,Z:%f\n",
				(sta_prom_geo1_X / lta_prom_geo1_X) ,
				(sta_prom_geo1_Y / lta_prom_geo1_Y) ,
				(sta_prom_geo1_Z / lta_prom_geo1_Z) );

		sta_sum_geo1_X = 0;
		sta_sum_geo1_Y = 0;
		sta_sum_geo1_Z = 0;
		sta_sum_geo2_X = 0;
		sta_sum_geo2_Y = 0;
		sta_sum_geo2_Z = 0;

		i+=sta_size;
	}

//	double rms_geo1_X = sqrt(sum_square_geo1_X / num_samples);
//	double rms_geo1_Y = sqrt(sum_square_geo1_Y / num_samples);
//	double rms_geo1_Z = sqrt(sum_square_geo1_Z / num_samples);
//	double rms_geo2_X = sqrt(sum_square_geo2_X / num_samples);
//	double rms_geo2_Y = sqrt(sum_square_geo2_Y / num_samples);
//	double rms_geo2_Z = sqrt(sum_square_geo2_Z / num_samples);
//
//	printf("rms_geo1_X: %f,rms_geo1_Y: %f,rms_geo1_Z: %f\n",rms_geo1_X,rms_geo1_Y,rms_geo1_Z);
//	printf("rms_geo2_X: %f,rms_geo2_Y: %f,rms_geo1_Z: %f\n",rms_geo2_X,rms_geo2_Y,rms_geo2_Z);
}


void app_main(void)
{
	int available_block_for_new_task = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
	ESP_LOGW("MAIN", "Available Memory For Tasks = %d\n", available_block_for_new_task);

	int size = sizeof(CHANNEL_1_Y) / sizeof(int);
	printf("array size: %d\n",size);

	int *buffer1 = (int*)malloc(lta_size*sizeof(int));
	int *buffer2 = (int*)malloc(lta_size*sizeof(int));
	int *buffer3 = (int*)malloc(lta_size*sizeof(int));
	int *buffer4 = (int*)malloc(lta_size*sizeof(int));
	int *buffer5 = (int*)malloc(lta_size*sizeof(int));
	int *buffer6 = (int*)malloc(lta_size*sizeof(int));
	int i = 0;
	while (i<(size)){
		printf("%d\n",i);
		memcpy(&buffer1[0],&CHANNEL_1_X[i],lta_size*sizeof(int));
		memcpy(&buffer2[0],&CHANNEL_1_Y[i],lta_size*sizeof(int));
		memcpy(&buffer3[0],&CHANNEL_1_Z[i],lta_size*sizeof(int));
		memcpy(&buffer4[0],&CHANNEL_2_X[i],lta_size*sizeof(int));
		memcpy(&buffer5[0],&CHANNEL_2_Y[i],lta_size*sizeof(int));
		memcpy(&buffer6[0],&CHANNEL_2_Z[i],lta_size*sizeof(int));
//		printf("buffer1: %d\n",buffer1[0]);
//		printf("buffer1: %d\n",buffer1[999]);
		event_detection(buffer1,buffer2,buffer3,buffer4,buffer5,buffer6);
		i+=lta_size;
	}
//

//	while (true) {
//        printf("Hello from app_main!\n");
//        sleep(1);
//    }
}
