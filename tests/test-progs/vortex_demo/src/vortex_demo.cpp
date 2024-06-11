#include <stdint.h>
#include <stdio.h>
// Define the addresses of interest in the GPU memory
#define DATA_ADDR ((volatile uint32_t *)0xa00000)
#define START_ADDR ((volatile uint32_t *)0xa01000)
#define DONE_ADDR ((volatile uint32_t *)0xa02000)

#define PAYLOAD 0xDEADBEEF

int main() {
    // We need to initialize the done signal to 0.
    // The GPU will set it to 1 when it's done.
    // Interestingly, the CPU can write to arbitrary addresses in the GPU, but
    // can only read from addresses it's previously written to.
    *DONE_ADDR = 0;

    uint32_t payload = PAYLOAD;

    printf("CPU: Commencing write of payload 0x%x to GPU's DATA address.\n",
           payload);
    *DATA_ADDR = payload;

    printf("CPU: Commencing write of 1 to GPU's START address.\n");

    *START_ADDR = 1;
    uint32_t done_signal = *DONE_ADDR;
    while (done_signal == 0) {
        printf("CPU: Waiting for GPU to finish. Done signal: %d\n",
               done_signal);
        done_signal = *DONE_ADDR;
    }

    printf("CPU: Detect that the GPU has finished with done signal: %d\n",
           done_signal);
    printf("CPU: Operation complete.\n");

    return 0;
}
