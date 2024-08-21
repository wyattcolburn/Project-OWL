#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "cdp.h"
#include "sx1262.h"
#include "helpFunctions.h"
#include "redis.h"

const char *queue_name = "myqueue";
const char *queue_name_2 = "rxqueue";
// Function prototypes
void *redis_thread_func(void *ptr);
void *lora_thread_func(void *ptr);
int chip_handle;
int spi_handle;
int tx_mode_flag = 0;
void get_pending_messages(redisContext *c, const char *stream_name, const char *group_name, const char *consumer_name) {
    // XPENDING to get the overview
    redisReply *pending_reply = (redisReply *)redisCommand(c, "XPENDING %s %s", stream_name, group_name);

    if (pending_reply == NULL || pending_reply->type != REDIS_REPLY_ARRAY || pending_reply->elements != 4) {
        printf("Error retrieving pending messages or no pending messages.\n");
        if (pending_reply) freeReplyObject(pending_reply);
        return;
    }

    printf("Total Pending: %lld\n", pending_reply->element[0]->integer);
    if (pending_reply->element[0]->integer > 0) {
        printf("Smallest Pending ID: %s\n", pending_reply->element[1]->str);
        printf("Largest Pending ID: %s\n", pending_reply->element[2]->str);

        // Now use XREADGROUP to read the actual pending messages
        redisReply *messages_reply = (redisReply *)redisCommand(c, "XREADGROUP GROUP %s %s STREAMS %s %s", group_name, consumer_name, stream_name, pending_reply->element[1]->str);
	printf("string from xreadgroup: %s\n", pending_reply->element[1]->str);
        if (messages_reply == NULL || messages_reply->type != REDIS_REPLY_ARRAY || messages_reply->elements == 0) {
            printf("No messages retrieved.\n");
            if (messages_reply) freeReplyObject(messages_reply);
            freeReplyObject(pending_reply);
            return;
        }

        for (size_t i = 0; i < messages_reply->elements; i++) {
            redisReply *stream = messages_reply->element[i];
            redisReply *messages = stream->element[1];

            for (size_t j = 0; j < messages->elements; j++) {
                redisReply *message = messages->element[j];
                printf("Message ID: %s\n", message->element[0]->str);  // Message ID

                // Expecting one key-value pair in the message
                redisReply *fields = message->element[1];
                if (fields->type == REDIS_REPLY_ARRAY && fields->elements == 2) {
                    printf("Key: %s, Value: %s\n", fields->element[0]->str, fields->element[1]->str);
                } else {
                    printf("Unexpected message structure.\n");
                }
            }
        }

        freeReplyObject(messages_reply);
    }
    freeReplyObject(pending_reply);
}
int main() {
    puts("hello world");

	redisContext * c = redis_init("127.0.0.1", 6379);
	delete_stream(c, "mystream");
	// Clear the queue at startup
    redisCommand(c, "DEL %s", queue_name);
    redisCommand(c, "DEL %s", queue_name_2);
	//spi lora stuff
	chip_handle = lgpio_init();
	spi_handle = spiHandle(0, 0, 5000000, 0);
	gpio_init(chip_handle);
	factoryReset();


    // Create Redis and LoRa threads
    pthread_t redis_thread, lora_thread;

    int retVal = pthread_create(&redis_thread, NULL, redis_thread_func, NULL);
    int retVal1 = pthread_create(&lora_thread, NULL, lora_thread_func, NULL);

    if (retVal) {
        fprintf(stderr, "Error creating Redis thread: %d\n", retVal);
        return 1;
    }

    if (retVal1) {
        fprintf(stderr, "Error creating LoRa thread: %d\n", retVal1);
        return 1;
    }

    pthread_join(redis_thread, NULL);
    pthread_join(lora_thread, NULL);

    return 0;
}

void *lora_thread_func(void *ptr) {
    redisContext *c = redis_init("127.0.0.1", 6379); //not thread safe

    while (1) {  // Assuming you want to continuously check the queue
        rx_mode_attempt();
		printf("SENDING MESSAGE");
		int len = queue_len(c, queue_name);
        while (len > 0) {
            char taskBuffer[255];
			memset(taskBuffer, 0, 255);
			dequeue_task(c, queue_name, taskBuffer);
				uint16_t payload_len = strlen(taskBuffer)+ 1;
				send_packet((uint8_t* )taskBuffer, payload_len);
				printf("packet has been sent\n");
				len = queue_len(c, queue_name);			
        }
		tx_mode_flag = 0;
	//adding tasks to send to the CDP people 
	sleep(5);
    }
    redisFree(c);  // Clean up Redis context
    return NULL;
}


void *redis_thread_func(void *ptr) {
	redisContext *c = redis_init("127.0.0.1", 6379);

	const char *mystream = "mystream";
	const char *groupName = "loraGroup";
	const char *consumer_name = "consumer";
	char keyBuffer[100];
	char messageBuffer[255];
	char messageID[100];

	char keyBuffer_2[100];
	char messageBuffer_2[100];
	char messageID_2[100];
	create_consumer_group(c, mystream, groupName);
    
	while (1) {  // Assuming you want to continuously read from the stream
		puts("1111********************************");
		readStream(c, mystream, messageBuffer_2);
		puts("2222********************************");
		read_from_consumer_group(c, mystream, groupName, consumer_name, keyBuffer, messageBuffer, messageID);
		puts("3333********************************");
		/*check_pending_messages(c, mystream, groupName, messageID_2);*/
		puts("4444********************************");
		printf("printing out the buffers\n");
		printf("key buffers : %s\n", keyBuffer);
		printf("message buffers : %s\n", messageBuffer);
		if (keyBuffer[0] == '\0') {
			printf("No message received key buffer is empty\n");
		}

		//define keys
		//CDP_LORA
		//LORA_CDP
		//WEB_CDP
		//CDP_WEB
		else if(((strcmp(keyBuffer, "WEB_CDP") == 0) | 
				(strcmp(keyBuffer, "CDP_WEB") == 0)) | 
				(strcmp(keyBuffer, "LORA_CDP")==0)) {
				

			acknowledge_message(c, mystream, groupName, messageID);
			memset(keyBuffer, 0, sizeof(keyBuffer));
			memset(messageBuffer, 0, sizeof(messageBuffer));
			memset(messageID, 0, sizeof(messageID));
				}

		else if(strcmp(keyBuffer, "CDP_LORA") == 0) //if str are equal
		{
			printf("Key received: %s\n", keyBuffer);
			printf("Message Received: %s\n", messageBuffer);
			
			enqueue_task(c, queue_name, messageBuffer);
			acknowledge_message(c, mystream,groupName, messageID);
			check_pending_messages(c, mystream, groupName, messageID_2);
			print_queue(c, queue_name);
			tx_mode_flag = 1;

			printf("message string %s", messageBuffer);
			memset(keyBuffer, 0, sizeof(keyBuffer));
			memset(messageBuffer, 0, sizeof(messageBuffer));
			memset(messageID, 0, sizeof(messageID));

		}
		else {
			printf("key does not match format: %s\n", keyBuffer);
		}

		int rx_queue_len = queue_len(c, queue_name_2);
		if (rx_queue_len > 0) {
			puts("queue has contents");
			print_queue(c, queue_name_2);
			const char * key = "LORA_CDP"; //lora --> cdp
			const char * mystream = "mystream";
			char value[255];
			char response[10];
			dequeue_task(c, queue_name_2, value);
			publish(c, mystream, key, value, response);
		}
		else {
			puts("nothing in rx queue");
			}
		sleep(1);
	}
	redisFree(c);  // Clean up Redis context
	return NULL;
}


