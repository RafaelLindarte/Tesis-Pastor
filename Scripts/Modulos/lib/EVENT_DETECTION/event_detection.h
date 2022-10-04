#ifndef _EVENT_DETECTION_H_
#define _EVENT_DETECTION_H_
typedef struct
{
    double corrTotal;
    double promTotal;
} EventParams;

void event_detection_task( void *params );
#endif