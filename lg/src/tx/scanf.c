#include "stdint.h"
#include "stdio.h"
#include "string.h"
int main() {
    puts("What message would you like to send?");
    char messageBuffer[255];

    // Use fgets to read a whole line of input into the buffer
    if (fgets(messageBuffer, sizeof(messageBuffer), stdin) != NULL) {
        // Remove the newline character at the end if present
        size_t len = strlen(messageBuffer);
        if (len > 0 && messageBuffer[len - 1] == '\n') {
            messageBuffer[len - 1] = '\0';
        }

        // Print the message
        printf("You entered: ");
        for (int i = 0; i < len; i++) {
            printf("%c", messageBuffer[i]);
        }
        printf("\n");
    } else {
        printf("Error reading input\n");
    }

    return 0;
}

