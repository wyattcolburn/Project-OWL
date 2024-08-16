#ifndef __cplusplus
extern "C" {
#endif
#include <hiredis/hiredis.h>


redisContext * redis_init(const char * server, int port);
void publish(redisContext *redisConnect, const char *stream_name, const char *key, const char *value, char *response);

void readStream(redisContext * redis_connect, const char * stream_name, char * response);


void create_consumer_group(redisContext * c, const char *stream_name, const char *group_name);

void read_from_consumer_group(redisContext * c, const char *stream_name, const char *group_name, const char *consumer_name, char * key_buffer, char * message_buffer, char * messageID);

void acknowledge_message(redisContext * c, const char *stream_name, const char *group_name, const char *message_id);

void check_pending_messages(redisContext * c, const char *stream_name, const char *group_name);


void delete_stream(redisContext * c, const char *stream_name);

void enqueue_task(redisContext * c, const char * queue_name, const char *task);

void dequeue_task(redisContext * c, const char * queue_name, char * taskBuffer);

void print_queue(redisContext * c, const char * queue_name);

int queue_len(redisContext *c, const char * queue_name);

#ifdef __cplusplus
}
#endif


