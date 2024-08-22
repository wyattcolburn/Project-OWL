#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "cdp.h"
#include "sx1262.h"
#include "helpFunctions.h"
#include "redis.h"

int main(){

	puts("hello world");

	redisContext * c = redis_init("127.0.0.1", 6379);


	
	size_t maxMessages = 100;

	KeyValuePair * messageArray = malloc(maxMessages * sizeof(KeyValuePair));
	if (messageArray == NULL) {
		fprintf(stderr, "Mem allocation failed\n");
		return 1;
	}


	const char * stream_name = "mystream";	
	const char *group_name = "loraGroup";
	const char *consumer_name = "consumer";


	size_t totalMessages = 0;

	read_from_consumer_group_dynamic(c, stream_name, group_name, consumer_name, &messageArray, &totalMessages);

    if (messageArray != NULL && totalMessages > 0) {
        for (size_t i = 0; i < totalMessages; i++) {
            printf("Message #%zu - Key: %s, Value: %s\n", i, messageArray[i].key, messageArray[i].value);
            // Free the memory for each key-value pair
            free(messageArray[i].key);
            free(messageArray[i].value);
        }
        // Free the array itself
        free(messageArray);
    }


}
