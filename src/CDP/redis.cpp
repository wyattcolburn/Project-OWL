#include <iostream>
#include <string>
#include <cstring>
#include <hiredis/hiredis.h>
#include "redis.h"

redisContext* redis_init(const char* server, int port) {
    std::cout << "redis init" << std::endl;
    redisContext* c = redisConnect(server, port);
    if (c == NULL || c->err) {
        if (c) {
            std::cerr << "Connection error: " << c->errstr << std::endl;
        } else {
            std::cerr << "Connection error: can't allocate redis context" << std::endl;
        }
        exit(1);
    }
    std::cout << "connection established" << std::endl;
    return c;
}

void publish(redisContext *redisConnect, const char *stream_name, const char *key, const char *value, char *response) {
    if (stream_name == NULL || key == NULL || value == NULL || response == NULL) {
        return;
    }

    // Send the command to Redis using argument-based function
    redisReply *reply = (redisReply *)redisCommand(redisConnect, "XADD %s * %s %s", stream_name, key, value);

    if (reply == NULL) {
        printf("Command execution error\n");
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

    // Free the reply object
    freeReplyObject(reply);
}




void read_from_consumer_group(redisContext* c, const char* stream_name, const char* group_name, const char* consumer_name, char* key_buffer, char* message_buffer, char* messageID) {
    redisReply* reply = (redisReply*)redisCommand(c, "XREADGROUP GROUP %s %s STREAMS %s >", group_name, consumer_name, stream_name);

    if (reply == NULL) {
        std::cerr << "Command execution error" << std::endl;
        return;
    }

    if (reply->type == REDIS_REPLY_ARRAY && reply->elements > 0) {
        for (size_t i = 0; i < reply->elements; i++) {
            redisReply* stream = reply->element[i];
            redisReply* messages = stream->element[1];

            for (size_t j = 0; j < messages->elements; j++) {
                redisReply* message = messages->element[j];
                std::cout << "Message ID: " << message->element[0]->str << std::endl;
                std::strcat(messageID, message->element[0]->str);
                redisReply* fields = message->element[1];
                message_buffer[0] = '\0'; // Clear the buffer before concatenating the message

                // Extract and concatenate key-value pairs into the message buffer
                for (size_t k = 0; k < fields->elements; k += 2) {
                    std::strcat(key_buffer, fields->element[k]->str);
                    std::strcat(message_buffer, fields->element[k + 1]->str);
                    if (k + 2 < fields->elements) {
                        std::strcat(message_buffer, ", ");  // Separate key-value pairs with a comma
                    }
                }

                std::cout << "Message Content: " << message_buffer << std::endl;
            }
        }
    } else {
        std::cerr << "No messages found." << std::endl;
        std::memset(key_buffer, 0, 100);
        std::memset(message_buffer, 0, 100);
    }

    freeReplyObject(reply);
}

void readStream(redisContext* redis_connect, const char* stream_name, char* response) {
    if (stream_name == NULL || response == NULL) {
        return;
    }

    response[0] = '\0';  // Ensure the response buffer starts empty

    redisReply* reply = (redisReply*)redisCommand(redis_connect, "XRANGE %s - +", stream_name);

    if (reply->type == REDIS_REPLY_ARRAY) { 
        for (size_t i = 0; i < reply->elements; i++) {
            redisReply* message = reply->element[i];
            redisReply* fields = message->element[1];

            for (size_t j = 1; j < fields->elements; j += 2) {
                std::strcat(response, fields->element[j]->str); // Append the value to the response buffer

                if (j + 2 < fields->elements) {
                    std::strcat(response, ", ");  // Add a comma and space between multiple values
                }
            }
        }
    }
    freeReplyObject(reply);
}

void create_consumer_group(redisContext* c, const char* stream_name, const char* group_name) {
    redisReply* reply = (redisReply*)redisCommand(c, "XGROUP CREATE %s %s $ MKSTREAM", stream_name, group_name);

    if (reply == NULL) {
        std::cerr << "Command execution error" << std::endl;
        return;
    }

    if (reply->type == REDIS_REPLY_ERROR) {
        std::cerr << "ERROR: " << reply->str << std::endl;
    } else {
        std::cout << "Consumer group " << group_name << " created on stream " << stream_name << std::endl;
    }

    freeReplyObject(reply);
}

void acknowledge_message(redisContext* c, const char* stream_name, const char* group_name, const char* message_id) {
    redisReply* reply = (redisReply*)redisCommand(c, "XACK %s %s %s", stream_name, group_name, message_id);

    if (reply == NULL) {
        std::cerr << "Command execution error" << std::endl;
        return;
    }

    if (reply->type == REDIS_REPLY_INTEGER) {
        std::cout << "Acknowledged " << reply->integer << " messages" << std::endl;
    } else {
        std::cerr << "Failed to acknowledge message" << std::endl;
    }

    freeReplyObject(reply);
}

void check_pending_messages(redisContext* c, const char* stream_name, const char* group_name) {
    redisReply* reply = (redisReply*)redisCommand(c, "XPENDING %s %s", stream_name, group_name);

    if (reply == NULL) {
        std::cerr << "Command execution error" << std::endl;
        return;
    }

    if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 4) {
        std::cout << "Pending count: " << reply->element[0]->integer << std::endl;
        std::cout << "Smallest Pending ID: " << reply->element[1]->str << std::endl;
        std::cout << "Largest Pending ID: " << reply->element[2]->str << std::endl;
        std::cout << "Consumers with pending messages: " << reply->element[3]->elements << std::endl;
    } else {
        std::cout << "No pending messages." << std::endl;
    }

    freeReplyObject(reply);
}

void delete_stream(redisContext* c, const char* stream_name) {
    redisReply* reply = (redisReply*)redisCommand(c, "DEL %s", stream_name);

    if (reply == NULL) {
        std::cerr << "Command execution error" << std::endl;
    } else if (reply->type == REDIS_REPLY_INTEGER && reply->integer == 1) {
        std::cout << "Stream '" << stream_name << "' deleted successfully." << std::endl;
    } else {
        std::cout << "Stream '" << stream_name << "' does not exist or could not be deleted." << std::endl;
    }

    freeReplyObject(reply);
}

void enqueue_task(redisContext* c, const char* queue_name, const char* task) {
    redisReply* reply = (redisReply*)redisCommand(c, "RPUSH %s %s", queue_name, task);
    if (reply->type == REDIS_REPLY_INTEGER) {
        std::cout << "Task added to queue '" << queue_name << "'. Queue length: " << reply->integer << std::endl;
    }
    freeReplyObject(reply);
}

void dequeue_task(redisContext* c, const char* queue_name, char* taskBuffer) {
    redisReply* reply = (redisReply*)redisCommand(c, "LPOP %s", queue_name);
    if (reply->type == REDIS_REPLY_STRING) {
        std::cout << "Dequeued task: " << reply->str << std::endl;
        std::strcat(taskBuffer, reply->str);
    } else {
        std::cout << "Queue is empty or command failed." << std::endl;
    }
    freeReplyObject(reply);
}

void print_queue(redisContext* c, const char* queue_name) {
    redisReply* reply = (redisReply*)redisCommand(c, "LRANGE %s 0 -1", queue_name);

    if (reply == NULL) {
        std::cerr << "Command execution error" << std::endl;
        return;
    }

    if (reply->type == REDIS_REPLY_ARRAY) {
        std::cout << "Queue '" << queue_name << "' contains " << reply->elements << " elements:" << std::endl;
        for (size_t i = 0; i < reply->elements; i++) {
            std::cout << (i + 1) << ": " << reply->element[i]->str << std::endl;
        }
    } else {
        std::cout << "Queue '" << queue_name << "' is empty or command failed." << std::endl;
    }

    freeReplyObject(reply);
}

int queue_len(redisContext* c, const char* queue_name) {
    redisReply* reply = (redisReply*)redisCommand(c, "LLEN %s", queue_name);

    if (reply == NULL) {
        std::cerr << "Command execution error" << std::endl;
        return -1;
    }

    int length = 0;

    if (reply->type == REDIS_REPLY_INTEGER) {
        length = static_cast<int>(reply->integer);
    } else {
        std::cerr << "Unexpected reply type: " << reply->type << std::endl;
    }

    freeReplyObject(reply);
    return length;
}

