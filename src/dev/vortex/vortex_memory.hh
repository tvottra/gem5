#ifndef __VORTEX_MEMORY_HH__
#define __VORTEX_MEMORY_HH__
#include "mem/mem_interface.hh"
#include "mem/nvm_interface.hh"
#include "params/VortexMemory.hh"

namespace gem5 {
namespace memory {
class VortexMemory : public memory::NVMInterface {
   private:
    static constexpr int MAX_PAYLOAD_SZ = 1024;
    void doWork();

    const Addr data_addr;
    const Addr start_addr;
    const Addr done_addr;
    const int data_sz_bytes;
    const int DONE_SIGNAL = 1;
    const Tick latency_of_work;

    EventFunctionWrapper event;

    uint32_t data[MAX_PAYLOAD_SZ];  // 4KB of data

   public:
    VortexMemory(const VortexMemoryParams& params);

    void access(PacketPtr pkt) override;
};
}  // namespace memory

}  // namespace gem5

#endif  // __VORTEX_MEMORY_HH__
