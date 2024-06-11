
#include "dev/vortex/vortex_memory.hh"

#include "base/trace.hh"
#include "debug/VortexMemory.hh"
#include "mem/mem_interface.hh"
#include "mem/nvm_interface.hh"
#include "mem/packet.hh"

namespace gem5 {

namespace memory {
VortexMemory::VortexMemory(const VortexMemoryParams& params)
    : NVMInterface(params),
      data_addr(params.data_addr),
      start_addr(params.start_addr),
      done_addr(params.done_addr),
      data_sz_bytes(params.data_sz),
      latency_of_work(params.latency_of_work),
      event([this] { doWork(); }, name() + ".event") {}

void VortexMemory::doWork() {
    uint8_t* data_paddr_in_host = toHostAddr(data_addr);
    std::memcpy(data, data_paddr_in_host, data_sz_bytes);
    DPRINTF(VortexMemory, "GPU: Data read from the data address: %#x\n",
            data[0]);

    DPRINTF(VortexMemory,
            "GPU: Writing to the done address to indicate completion of "
            "operation.\n");

    uint8_t* done_paddr_in_host = toHostAddr(done_addr);
    std::memcpy(done_paddr_in_host, &DONE_SIGNAL, sizeof(DONE_SIGNAL));
    DPRINTF(VortexMemory,
            "GPU: Done signal has been written to the done address.\n");
}

void VortexMemory::access(PacketPtr pkt) {
    AbstractMemory::access(pkt);
    DPRINTF(VortexMemory, "GPU: GPU memory is being accessed by the CPU.\n");
    if (pkt->isWrite() && pkt->getAddr() == start_addr) {
        const uint32_t* payload_ptr = pkt->getConstPtr<uint32_t>();
        const uint32_t payload = *payload_ptr;
        DPRINTF(VortexMemory,
                "GPU: Detect that the CPU has written to the start address "
                "with payload %#x.\n",
                payload);

        if (payload != 0) {
            DPRINTF(VortexMemory, "GPU: Starting operation!\n");
            schedule(event, curTick() + latency_of_work);
        }
    }
}
}  // namespace memory
}  // namespace gem5
