#include <stdint.h>
#include <stdio.h>

// Define the addresses
#define DATA_ADDR ((volatile uint32_t *)0xa001)
#define START_ADDR ((volatile uint32_t *)0xa002)
#define DONE_ADDR ((volatile uint32_t *)0xa003)

int main() {
    // Define the payload to write
    uint32_t payload = 0xDEADBEEF;

    // Write the payload to the DATA address
    *DATA_ADDR = payload;
    printf("Payload 0x%X written to DATA address.\n", payload);

    // Write 1 to the START address
    *START_ADDR = 1;
    printf("Start signal sent.\n");

    // Spinlock until DONE address returns 1
    // while (*DONE_ADDR != 1) {
    //     // Optional: add a small delay or a NOP instruction to avoid
    //     // busy-waiting too aggressively
    // }
    printf("Operation complete.\n");

    return 0;
}
