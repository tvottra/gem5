cd tests/test-progs/vortex_demo/src/
make
cd ../../../../
build/X86/gem5.opt --debug-flags=VortexMemory configs/vortex/simple_x86.py
