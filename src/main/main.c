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
	char messageBuffer[100];
	char messageID[100];

	create_consumer_group(c, mystream, groupName);
	puts("before reading any messages, lets see # pending messages\n");
	check_pending_messages(c, mystream, groupName);
    
	while (1) {  // Assuming you want to continuously read from the stream
		read_from_consumer_group(c, mystream, groupName, consumer_name, keyBuffer, messageBuffer, messageID);
		check_pending_messages(c, mystream, groupName);
		if (keyBuffer[0] == '\0') {
			/*printf("No message received key buffer is empty\n");*/
		}
		else if(strcmp(keyBuffer, "CDP_LORA") == 0) //if str are equal
		{
			printf("Key received: %s\n", keyBuffer);
			printf("Message Received: %s\n", messageBuffer);
			
			enqueue_task(c, queue_name, messageBuffer);
			acknowledge_message(c, mystream,groupName, messageID);
			check_pending_messages(c, mystream, groupName);
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
			const char * key = "LORA_CDP";
			const char * mystream = "mystream";
			char value[255];
			char response[10];
			dequeue_task(c, queue_name_2, value);
			publish(c, mystream, key, value, response);
		}	
		sleep(1);
	}
	redisFree(c);  // Clean up Redis context
	return NULL;
}


