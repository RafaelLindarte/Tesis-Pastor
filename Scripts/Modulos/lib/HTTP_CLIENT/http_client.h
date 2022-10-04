#ifndef _HTTP_CLIENT_
#define _HTTP_CLIENT_


void http_client( void *params );

typedef struct
{
    char *key;
    char *val;
} Header;

typedef struct
{
    char *key;
    char *val;
} Body;

typedef enum
{
    Get,
    Post
} HttpMethod;

struct RequestParams
{
    void (*OnGotData)(char *incomingBuffer, char *output);
    char message[300];
    Header header[2];
    int headerCount;
    HttpMethod method;
    char *body;
    int status;
};

#endif
