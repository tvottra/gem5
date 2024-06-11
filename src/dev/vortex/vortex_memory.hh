#ifndef __VORTEX_MEMORY_HH__
#define __VORTEX_MEMORY_HH__
#include "mem/mem_interface.hh"
#include "mem/nvm_interface.hh"
#include "params/VortexMemory.hh"

namespace gem5 {
namespace memory {
class VortexMemory : public memory::NVMInterface {
   private:
    const Addr data_addr;
    const Addr start_addr;
    const Addr done_addr;
    const int data_sz_bytes;
    const int done_payload = 1;

    uint32_t data[1024];  // 4KB of data

   public:
    VortexMemory(const VortexMemoryParams& params);

    void access(PacketPtr pkt) override;
    MemPacket* decodePacket(const PacketPtr pkt, Addr pkt_addr, unsigned size,
                            bool is_read, uint8_t pseudo_channel) override;
};
}  // namespace memory

}  // namespace gem5

#endif  // __VORTEX_MEMORY_HH__
