#include <string.h>
#include <stdlib.h>
#include <ctype.h>


void processHexPacket(const char *input, unsigned char *output, int *outputLen) {
    int i = 0, j = 0;
    char temp[3] = {0}; // Temporary buffer to hold a hex pair

    while (input[i] != '\0') {
        if (isspace(input[i])) {
            i++;
            continue;
        }

        // Copy the next two characters as a hex pair
        temp[0] = input[i++];
        temp[1] = input[i++];

        // Convert the hex pair to an integer
        output[j++] = (unsigned char)strtol(temp, NULL, 16);
    }

    *outputLen = j;
}


