#include <stdint.h>
#include <stdio.h>

// Define the addresses
#define DATA_ADDR ((volatile uint32_t *)0xa00000)
#define START_ADDR ((volatile uint32_t *)0xa01000)
#define DONE_ADDR ((volatile uint32_t *)0xa02000)

int main() {
    // Define the payload to write
    uint32_t payload = 0xDEADBEEF;

    // Write the payload to the DATA address
    printf("CPU: commencing write of payload 0x%X to GPU's DATA address.\n",
           payload);
    *DATA_ADDR = payload;

    // Write 1 to the START address
    printf("CPU: commencing write of 1 to GPU's START address.\n");
    *START_ADDR = 1;

    // Spinlock until DONE address returns 1
    // while (*DONE_ADDR != 1) {
    //     // Optional: add a small delay or a NOP instruction to avoid
    //     // busy-waiting too aggressively
    // }
    printf("Operation complete.\n");

    return 0;
}
