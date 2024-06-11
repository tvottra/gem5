#include <stdint.h>
#include <stdio.h>
// Define the addresses
#define DATA_ADDR ((volatile uint32_t *)0xa00000)
#define START_ADDR ((volatile uint32_t *)0xa01000)
#define DONE_ADDR ((volatile uint32_t *)0xa02000)

int main() {
    *DONE_ADDR = 0;
    // Define the payload to write
    uint32_t payload = 0xDEADBEEF;

    // Write the payload to the DATA address
    printf("CPU: commencing write of payload 0x%X to GPU's DATA address.\n",
           payload);
    *DATA_ADDR = payload;

    // Write 1 to the START address
    printf("CPU: commencing write of 1 to GPU's START address.\n");

    uint32_t done_signal = *DONE_ADDR;
    while (done_signal == 0) {
        printf("CPU: Waiting for GPU to finish. Done signal: %d\n",
               done_signal);
        done_signal = *DONE_ADDR;
        *START_ADDR = 1;
    }

    printf("Operation complete.\n");

    return 0;
}
