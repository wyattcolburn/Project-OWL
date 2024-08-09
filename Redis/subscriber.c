#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hiredis/hiredis.h>
void printFunc(char * inputString);
int main(void) {
    // Connect to Redis server
    redisContext *c = redisConnect("127.0.0.1", 6379);
    if (c == NULL || c->err) {
        if (c) {
            printf("Connection error: %s\n", c->errstr);
            redisFree(c);
        } else {
            printf("Connection error: can't allocate redis context\n");
        }
        exit(1);
    }

    // Subscribe to the channel
    const char *channel = "mychannel";
    redisReply *reply = redisCommand(c, "SUBSCRIBE %s", channel);
    freeReplyObject(reply);

    // Listen for messages
    while (redisGetReply(c, (void**)&reply) == REDIS_OK) {
        if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 3) {
            if (strcmp(reply->element[0]->str, "message") == 0) {
                //printf("Received message: %s\n", reply->element[2]->str);
				printFunc(reply->element[2]->str);
            }
        }
        freeReplyObject(reply);
    }

    // Disconnect and free the context
    redisFree(c);
    
    return 0;
}
void printFunc(char * inputString){
	puts("func call");
	printf("%s\n", inputString);
}
