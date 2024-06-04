
from m5.objects import MemInterface

class VortexMemory(MemInterface):
    type = 'VortexMemory'
    cxx_header = 'dev/vortex/VortexMemory.hh'
    cxx_class = 'gem5::VortexMemory'
    latency = '10ns'
    bandwidth = '10GB/s'
    device_size = '512MiB'


