
#include "dev/vortex/vortex_memory.hh"

#include "base/trace.hh"
#include "debug/VortexMemory.hh"
#include "mem/mem_interface.hh"
#include "mem/nvm_interface.hh"
namespace gem5 {

namespace memory {
VortexMemory::VortexMemory(const VortexMemoryParams& params)
    : NVMInterface(params) {}

MemPacket* VortexMemory::decodePacket(const PacketPtr pkt, Addr pkt_addr,
                                      unsigned size, bool is_read,
                                      uint8_t pseudo_channel) {
    // decode the address based on the address mapping scheme, with
    // Ro, Ra, Co, Ba and Ch denoting row, rank, column, bank and
    // channel, respectively
    uint8_t rank;
    uint8_t bank;
    // use a 64-bit unsigned during the computations as the row is
    // always the top bits, and check before creating the packet
    uint64_t row;

    // Get packed address, starting at 0
    Addr addr = getCtrlAddr(pkt_addr);

    // truncate the address to a memory burst, which makes it unique to
    // a specific buffer, row, bank, rank and channel
    addr = addr / burstSize;

    // we have removed the lowest order address bits that denote the
    // position within the column
    if (addrMapping == enums::RoRaBaChCo || addrMapping == enums::RoRaBaCoCh) {
        // the lowest order bits denote the column to ensure that
        // sequential cache lines occupy the same row
        addr = addr / burstsPerRowBuffer;

        // after the channel bits, get the bank bits to interleave
        // over the banks
        bank = addr % banksPerRank;
        addr = addr / banksPerRank;

        // after the bank, we get the rank bits which thus interleaves
        // over the ranks
        rank = addr % ranksPerChannel;
        addr = addr / ranksPerChannel;

        // lastly, get the row bits, no need to remove them from addr
        row = addr % rowsPerBank;
    } else if (addrMapping == enums::RoCoRaBaCh) {
        // with emerging technologies, could have small page size with
        // interleaving granularity greater than row buffer
        if (burstsPerStripe > burstsPerRowBuffer) {
            // remove column bits which are a subset of burstsPerStripe
            addr = addr / burstsPerRowBuffer;
        } else {
            // remove lower column bits below channel bits
            addr = addr / burstsPerStripe;
        }

        // start with the bank bits, as this provides the maximum
        // opportunity for parallelism between requests
        bank = addr % banksPerRank;
        addr = addr / banksPerRank;

        // next get the rank bits
        rank = addr % ranksPerChannel;
        addr = addr / ranksPerChannel;

        // next, the higher-order column bites
        if (burstsPerStripe < burstsPerRowBuffer) {
            addr = addr / (burstsPerRowBuffer / burstsPerStripe);
        }

        // lastly, get the row bits, no need to remove them from addr
        row = addr % rowsPerBank;
    } else
        panic("Unknown address mapping policy chosen!");

    assert(rank < ranksPerChannel);
    assert(bank < banksPerRank);
    assert(row < rowsPerBank);
    assert(row < Bank::NO_ROW);

    DPRINTF(VortexMemory, "Address: %#x Rank %d Bank %d Row %d\n", pkt_addr,
            rank, bank, row);
    DPRINTF(VortexMemory, "%s\n", getAddrRange().to_string());

    // create the corresponding memory packet with the entry time and
    // ready time set to the current tick, the latter will be updated
    // later
    uint16_t bank_id = banksPerRank * rank + bank;

    return new MemPacket(pkt, is_read, false, pseudo_channel, rank, bank, row,
                         bank_id, pkt_addr, size);
}
}  // namespace memory
}  // namespace gem5
