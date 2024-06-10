.PHONY: all
all:
	scons build/X86/gem5.opt -j 12

clean:
	python3 `which scons` --clean --no-cache
