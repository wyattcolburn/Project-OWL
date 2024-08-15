#include "redis.h"



#include <iostream>
void myFunction() {
	std::cout << "I just got executed!";
}


int main(){
	redisContext * c = redis_init("127.0.0.1", 6379);
	const char* mystream = "mystream";
	const char* key = "cdp_lora";
	const char* value = "hello world";

	  // Use a single buffer to receive the response
    char response[256];  // Adjust the size according to your needs
    publish(c, mystream, key, value, response);


	return 0;


}