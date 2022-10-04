#ifndef _INTERNAL_STORAGE_H
#define _INTERNAL_STORAGE_H


typedef struct
{
  int id;
  char dateTimeBuffer[50];
  char type[50];
  char data[50];
} nvs_event;


void internal_storage( void *params );
#endif