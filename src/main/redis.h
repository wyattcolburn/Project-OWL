#ifndef REDIS_H
#define REDIS_H
#include <hiredis/hiredis.h>


redisContext * redis_init(char * server, int port);
void publish(redisContext *redisConnect, const char *stream_name, const char *key, const char *value, char *response);
void readStream(redisContext * redis_connect, const char * stream_name, char * response);


#endif


