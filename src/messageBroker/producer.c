#include <stdio.h>
#include <stdlib.h>
#include <hiredis/hiredis.h>

int main() {
    // Connect to Redis
    redisContext *c = redisConnect("127.0.0.1", 6379);
    if (c == NULL || c->err) {
        if (c) {
            printf("Error: %s\n", c->errstr);
            redisFree(c);
        } else {
            printf("Can't allocate redis context\n");
        }
        exit(1);
    }

    // Add messages to a stream
    const char *stream_name = "mystream";
    redisReply *reply = (redisReply *)redisCommand(c, "XADD %s * key1 value1 key2 value2", stream_name);

    if (reply->type == REDIS_REPLY_ERROR) {
        printf("Error: %s\n", reply->str);
    } else {
        printf("Message added with ID: %s\n", reply->str);
    }
    freeReplyObject(reply);

    // Clean up
    redisFree(c);
    return 0;
}

