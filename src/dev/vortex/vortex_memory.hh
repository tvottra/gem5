#ifndef __VORTEX_MEMORY_HH__
#define __VORTEX_MEMORY_HH__
#include "mem/mem_interface.hh"
#include "mem/nvm_interface.hh"
#include "params/VortexMemory.hh"

namespace gem5 {
namespace memory {
class VortexMemory : public memory::NVMInterface {
   public:
    VortexMemory(const VortexMemoryParams& params);

    memory::MemPacket* decodePacket(const PacketPtr pkt, Addr pkt_addr,
                                    unsigned size, bool is_read,
                                    uint8_t pseudo_channel) override;
};
}  // namespace memory

}  // namespace gem5

#endif  // __VORTEX_MEMORY_HH__
