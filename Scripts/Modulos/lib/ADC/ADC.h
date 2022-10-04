#ifndef _ADC_H_
#define _ADC_H_
typedef struct
{
    double corrTotal;
    double promTotal;
} EventParams;

void adc_task( void *params );
#endif