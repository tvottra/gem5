
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
      data_sz_bytes(params.data_sz) {}

void VortexMemory::access(PacketPtr pkt) {
    AbstractMemory::access(pkt);
    if (pkt->isWrite() && pkt->getAddr() == start_addr) {
        const uint32_t* payload_buf = pkt->getConstPtr<uint32_t>();
        const uint32_t payload = payload_buf[0];
        DPRINTF(VortexMemory, "Start address written to with payload %#x\n",
                payload);
        if (payload != 0) {
            DPRINTF(VortexMemory, "Starting operation!\n");

            uint8_t* data_paddr_in_host = toHostAddr(data_addr);
            DPRINTF(VortexMemory, "Data paddress: %p\n",
                    static_cast<void*>(data_paddr_in_host));

            std::memcpy(data, data_paddr_in_host, data_sz_bytes);
            DPRINTF(VortexMemory, "Data read from data address: %#x\n",
                    data[0]);

            DPRINTF(VortexMemory, "Writing to done address\n");
            // Write the completion signal to done_addr
            uint32_t done_signal = 1;  // Indicate completion
            uint8_t* done_paddr_in_host = toHostAddr(done_addr);
            std::memcpy(done_paddr_in_host, &done_signal, sizeof(done_signal));
            DPRINTF(VortexMemory, "Done signal written to done address\n");

            uint32_t done_value = *done_paddr_in_host;
            DPRINTF(VortexMemory, "Done value: %#x\n", done_value);
        }
    } else if (pkt->isWrite() && pkt->getAddr() == done_addr) {
        DPRINTF(VortexMemory, "Done address accessed\n");
    } else if (pkt->isWrite() && pkt->getAddr() == data_addr) {
        DPRINTF(VortexMemory, "Data address accessed\n");
    }
}

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

    if (pkt_addr == data_addr) {
        DPRINTF(VortexMemory, "Data address: %#x\n", pkt_addr);
    } else if (pkt_addr == start_addr) {
        DPRINTF(VortexMemory, "Start address: %#x\n", pkt_addr);
    } else if (pkt_addr == done_addr) {
        DPRINTF(VortexMemory, "Done address: %#x\n", pkt_addr);
    }

    // create the corresponding memory packet with the entry time and
    // ready time set to the current tick, the latter will be updated
    // later
    uint16_t bank_id = banksPerRank * rank + bank;

    return new MemPacket(pkt, is_read, false, pseudo_channel, rank, bank, row,
                         bank_id, pkt_addr, size);
}  // TODO: replace most of this with just a call to the parent's decodePacket
}  // namespace memory
}  // namespace gem5
