#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <hiredis/hiredis.h>

#include "cdp.h"
#include "sx1262.h"
#include "helpFunctions.h"
#include "redis.h"
void *redis_thread_func(void*ptr);
int main(){
	puts("hello world");
	pthread_t redis_thread, lora_thread;

	int retVal = pthread_create(&redis_thread, NULL, redis_thread_func, (void *) NULL);
	if (retVal) {
        fprintf(stderr, "Error creating Redis thread: %d\n", retVal);
        return 1;
    }
	pthread_join(redis_thread, NULL);



	return 0;
}


void *redis_thread_func(void * ptr){
	
	redisContext *c = redis_init("127.0.0.1", 6379);
	char buffer[100];
	
	readStream(c, "mystream", buffer); 

	return NULL;
}
