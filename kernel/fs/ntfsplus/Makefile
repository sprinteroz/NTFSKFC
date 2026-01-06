#
# NTFSPLUS Kernel Module Makefile
# Compiles ntfs-3g source into kernel module
#

# Module name
obj-m += ntfsplus.o

# Source files
ntfsplus-y := ntfsplus_main.o kernel_volume.o kernel_logging.o kernel_utils.o kernel_attrib.o kernel_runlist.o kernel_mft.o kernel_inode.o kernel_compression.o kernel_transaction.o kernel_cache.o

# Suppress incompatible pointer type warnings for now
EXTRA_CFLAGS += -Wno-error=incompatible-pointer-types

# Kernel build system
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# Default target
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# Clean target
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f *.o *.ko *.mod *.mod.c .*.cmd *.symvers modules.order
	rm -rf .tmp_versions

# Install target
install: all
	sudo $(MAKE) -C $(KDIR) M=$(PWD) modules_install
	sudo depmod -a

# Test compilation without installing
test_compile:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	@echo "NTFSPLUS kernel module compiled successfully!"
	@ls -la ntfsplus.ko

# Show module info
info:
	@echo "NTFSPLUS Kernel Module"
	@echo "======================"
	@echo "Version: 1.0.18"
	@echo "Source: NTFSKFC (ntfs-3g)"
	@echo "License: GPL"
	@echo ""
	@echo "Files:"
	@ls -la *.c *.h 2>/dev/null || echo "No source files found"
	@echo ""
	@echo "Kernel version: $(shell uname -r)"
	@echo "Build directory: $(KDIR)"

.PHONY: all clean install test_compile info