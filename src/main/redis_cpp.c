#include <stdio.h>
#include <string.h>
#include <hiredis/hiredis.h>
#include "redis.h"

redisContext * redis_init(const char * server, int port){
	puts("redis init");
	redisContext * c = redisConnect(server, port);
	if ( c == NULL || c->err) {
		if (c) {
			printf("Connection error: %s\n", c->errstr);
		}
		else {
			printf("Conenction error:can't allocate redis context\n");
		}
		exit(1);
	}
	puts("connection established");
	puts("end message");
	return c;
}
void publish(redisContext *redisConnect, const char *stream_name, const char *key, const char *value, char *response) {
    if (stream_name == NULL || key == NULL || value == NULL || response == NULL) {
        return;
    }

    // Calculate the required command length
    size_t command_len = 8 + strlen(stream_name) + 1 + strlen(key) + 1 + strlen(value) + 1; // "XADD " + stream name + " * " + key + value + null terminator

    // Allocate memory for the command
    char *command = (char*)malloc(command_len * sizeof(char));
    if (!command) {
        printf("Memory allocation error\n");
        return;
    }

    // Build the command string
    snprintf(command, command_len, "XADD %s * %s %s", stream_name, key, value);

    // Send the command to Redis
    redisReply *reply = (redisReply *)redisCommand(redisConnect, command);

    if (reply == NULL) {
        printf("Command execution error\n");
        free(command);
        return;
    }

    if (reply->type == REDIS_REPLY_ERROR) {
        printf("ERROR: %s\n", reply->str);
    } else if (reply->type == REDIS_REPLY_STRING || reply->type == REDIS_REPLY_STATUS) {
        strcpy(response, reply->str);
        printf("Message added with ID: %s\n", reply->str);
    } else if (reply->type == REDIS_REPLY_NIL) {
        printf("No response from Redis\n");
    } else {
        printf("Unexpected reply type: %d\n", reply->type);
    }

    // Free the reply and command memory
    freeReplyObject(reply);
    free(command);
}
void read_from_consumer_group(redisContext *c, const char *stream_name, const char *group_name, const char *consumer_name, char * key_buffer, char *message_buffer, char *messageID) {
    redisReply *reply = (redisReply *)redisCommand(c, "XREADGROUP GROUP %s %s STREAMS %s >", group_name, consumer_name, stream_name);

    if (reply == NULL) {
        printf("Command execution error\n");
        return;
    }

    if (reply->type == REDIS_REPLY_ARRAY && reply->elements > 0) {
        for (size_t i = 0; i < reply->elements; i++) {
            redisReply *stream = reply->element[i];
            redisReply *messages = stream->element[1];

            for (size_t j = 0; j < messages->elements; j++) {
                redisReply *message = messages->element[j];
                printf("Message ID: %s\n", message->element[0]->str);  // Message ID (if you need it)
				strcat(messageID, message->element[0]->str);
                redisReply *fields = message->element[1];
                message_buffer[0] = '\0'; // Clear the buffer before concatenating the message

                // Extract and concatenate key-value pairs into the message buffer
                for (size_t k = 0; k < fields->elements; k += 2) {

                    strcat(key_buffer, fields->element[k]->str);
                    strcat(message_buffer, fields->element[k + 1]->str);
					/*strcat(message_buffer, fields->element[k]->str);*/
                    /*strcat(message_buffer, ": ");*/
                    /*strcat(message_buffer, fields->element[k + 1]->str);*/
                    if (k + 2 < fields->elements) {
                        strcat(message_buffer, ", ");  // Separate key-value pairs with a comma
                    }
                }

                printf("Message Content: %s\n", message_buffer);  // Print or use the message content
            }
        }
    } else {
        printf("No messages found.\n");
		memset(key_buffer, 0, 100);
		memset(message_buffer, 0,100); 
    }

    freeReplyObject(reply);
}


void readStream(redisContext *redis_connect, const char *stream_name, char *response) {
    if (stream_name == NULL || response == NULL) {
        return;
    }

    // Initialize response buffer
    response[0] = '\0';  // Ensure the response buffer starts empty

    redisReply *reply = (redisReply *)redisCommand(redis_connect, "XRANGE %s - +", stream_name);

    if (reply->type == REDIS_REPLY_ARRAY) { 
        for (size_t i = 0; i < reply->elements; i++) {
            redisReply *message = reply->element[i];
            redisReply *fields = message->element[1];

            // Assuming the message is stored as key-value pairs, and you want the value (not the key)
            for (size_t j = 1; j < fields->elements; j += 2) {
                strcat(response, fields->element[j]->str); // Append the value to the response buffer

                if (j + 2 < fields->elements) {
                    strcat(response, ", ");  // Add a comma and space between multiple values
                }
            }
        }
    }
    freeReplyObject(reply);
}

void create_consumer_group(redisContext *c, const char *stream_name, const char *group_name) {
    redisReply *reply = (redisReply *)redisCommand(c, "XGROUP CREATE %s %s $ MKSTREAM", stream_name, group_name);

    if (reply == NULL) {
        printf("Command execution error\n");
        return;
    }

    if (reply->type == REDIS_REPLY_ERROR) {
        printf("ERROR: %s\n", reply->str);
    } else {
        printf("Consumer group %s created on stream %s\n", group_name, stream_name);
    }

    freeReplyObject(reply);
}
void acknowledge_message(redisContext *c, const char *stream_name, const char *group_name, const char *message_id) {
    redisReply *reply = (redisReply *)redisCommand(c, "XACK %s %s %s", stream_name, group_name, message_id);

    if (reply == NULL) {
        printf("Command execution error\n");
        return;
    }

    if (reply->type == REDIS_REPLY_INTEGER) {
        printf("Acknowledged %lld messages\n", reply->integer);
    } else {
        printf("Failed to acknowledge message\n");
    }

    freeReplyObject(reply);
}
void check_pending_messages(redisContext *c, const char *stream_name, const char *group_name) {
    redisReply *reply = (redisReply *)redisCommand(c, "XPENDING %s %s", stream_name, group_name);

    if (reply == NULL) {
        printf("Command execution error\n");
        return;
    }

    if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 4) {
        printf("Pending count: %lld\n", reply->element[0]->integer);
        printf("Smallest Pending ID: %s\n", reply->element[1]->str);
        printf("Largest Pending ID: %s\n", reply->element[2]->str);
        printf("Consumers with pending messages: %llu\n", reply->element[3]->elements);
    } else {
        printf("No pending messages.\n");
    }

    freeReplyObject(reply);
}

void delete_stream(redisContext * c, const char *stream_name){

	redisReply *reply = (redisReply *)redisCommand(c, "DEL %s", stream_name);

if (reply == NULL) {
    printf("Command execution error\n");
} else if (reply->type == REDIS_REPLY_INTEGER && reply->integer == 1) {
    printf("Stream '%s' deleted successfully.\n", stream_name);
} else {
    printf("Stream '%s' does not exist or could not be deleted.\n", stream_name);
}

freeReplyObject(reply);
}
void enqueue_task(redisContext *c, const char *queue_name, const char *task) {
    redisReply *reply = (redisReply *)redisCommand(c, "RPUSH %s %s", queue_name, task);
    if (reply->type == REDIS_REPLY_INTEGER) {
        printf("Task added to queue '%s'. Queue length: %lld\n", queue_name, reply->integer);
    }
    freeReplyObject(reply);
}

void dequeue_task(redisContext *c, const char *queue_name, char * taskBuffer) {
    redisReply *reply = (redisReply *)redisCommand(c, "LPOP %s", queue_name);
    if (reply->type == REDIS_REPLY_STRING) {
        printf("Dequeued task: %s\n", reply->str);
		strcat(taskBuffer, reply->str);
    } else {
        printf("Queue is empty or command failed.\n");
    }
    freeReplyObject(reply);
}
void print_queue(redisContext *c, const char *queue_name) {
    // Fetch all elements from the queue using LRANGE 0 -1
    redisReply *reply = (redisReply *)redisCommand(c, "LRANGE %s 0 -1", queue_name);

    if (reply == NULL) {
        printf("Command execution error\n");
        return;
    }

    if (reply->type == REDIS_REPLY_ARRAY) {
        printf("Queue '%s' contains %zu elements:\n", queue_name, reply->elements);
        for (size_t i = 0; i < reply->elements; i++) {
            printf("%zu: %s\n", i + 1, reply->element[i]->str);
        }
    } else {
        printf("Queue '%s' is empty or command failed.\n", queue_name);
    }

    freeReplyObject(reply);
}

int queue_len(redisContext *c, const char *queue_name) {
    // Execute the LLEN command to get the length of the list
    redisReply *reply = (redisReply *)redisCommand(c, "LLEN %s", queue_name);

    if (reply == NULL) {
        printf("Command execution error\n");
        return -1;  // Return -1 to indicate an error
    }

    int length = 0;

    if (reply->type == REDIS_REPLY_INTEGER) {
        length = (int)reply->integer;  // The length of the queue
    } else {
        printf("Unexpected reply type: %d\n", reply->type);
    }

    freeReplyObject(reply);
    return length;
}
