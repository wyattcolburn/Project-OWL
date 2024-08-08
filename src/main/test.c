#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define INPUT_SIZE 95  // Adjust based on the actual length of your input string

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

int main() {
    const char *hexPacket = "12 34 56 78 91 01 11 23 20 19 18 17 16 15 14 13 12 34 56 78 12 21 99 98 76 54 32 48 65 6C 6C 6F 20 57 6F 72 6C 64 20 68 6F 77 20 64 6F 20 79 6F 75 20 64 6F";
    unsigned char processedData[INPUT_SIZE / 2];
    int processedDataLen;

    processHexPacket(hexPacket, processedData, &processedDataLen);

    printf("Processed Data:\n");
    for (int i = 0; i < 8; i++) {
        printf("%02X", processedData[i]);
    }
    printf("\n");

    return 0;
}

