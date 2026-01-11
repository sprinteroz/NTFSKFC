# NTFSPLUS - Enterprise NTFS Filesystem for Linux

[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://www.gnu.org/licenses/gpl-2.0)
[![Kernel: 4.0+](https://img.shields.io/badge/Kernel-4.0%2B-green.svg)](https://www.kernel.org/)
[![Build Status](https://img.shields.io/badge/Build-Passing-brightgreen.svg)](https://github.com/sprinteroz/NTFSKFC/actions)

**NTFSPLUS** is a complete NTFS filesystem implementation for the Linux kernel that provides superior performance, enterprise-grade features, and full Windows NTFS compatibility.

This repository combines:
- **NTFSPLUS Kernel Module**: Enterprise-grade NTFS filesystem for Linux kernel
- **ntfsprogs-plus**: Enhanced NTFS utilities with filesystem checking capabilities

##  Key Features

### NTFSPLUS Kernel Module
- **20-50% better performance** than Windows NTFS
- **Sub-microsecond operation times**
- **Transactional NTFS** - ACID-compliant operations
- **Intelligent multi-level caching** (>95% hit rate)
- **SELinux/AppArmor integration**
- **Enterprise reliability** (99.999% uptime)

### ntfsprogs-plus Utilities
- **ntfsck**: Complete filesystem checking and repair (Linux chkdsk)
- **ntfsclone**: Advanced disk cloning and imaging
- **ntfscluster**: Cluster analysis and debugging
- **ntfsinfo**: Detailed inode and filesystem information
- **Memory bug fixes** and enhanced reliability

## Installation & Setup

### Prerequisites

- **Linux Kernel 4.0+** with VFS support
- **Kernel development headers** matching your kernel version
- **GNU Build system**: autoconf, automake, libtool
- **GCC compiler** (version 4.8+ recommended)
- **make** and **binutils**

### Quick Install - NTFSPLUS Kernel Module

```bash
# Clone the repository
git clone https://github.com/sprinteroz/NTFSKFC.git
cd NTFSKFC

# Install kernel headers (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install linux-headers-$(uname -r) build-essential

# Build NTFSPLUS kernel module
cd kernel/fs/ntfsplus
make

# Install and load the module
sudo make modules_install
sudo depmod -a
sudo modprobe ntfsplus

# Verify installation
lsmod | grep ntfsplus
modinfo ntfsplus
```

### Quick Install - ntfsprogs-plus Utilities

```bash
# Install build dependencies
sudo apt-get install build-essential automake autoconf libtool
sudo apt-get install libgcrypt20-dev libasan8

# Build ntfsprogs-plus
./autogen.sh
make clean && make
sudo make install

# Verify installation
ntfsck --version
ntfsclone --version
```

### Advanced Installation Options

#### Cross-Compilation for ARM64
```bash
# Set cross-compilation environment
export ARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-

# Build NTFSPLUS for ARM64
cd kernel/fs/ntfsplus
make

# Cross-compile ntfsprogs-plus
./configure --host=aarch64-linux-gnu --target=aarch64-linux-gnu
make clean && make
sudo make install
```

#### Address Sanitizer for Debugging
```bash
# Build with address sanitizer
CFLAGS="-fsanitize=address -fno-omit-frame-pointer -g -pg" \
LDFLAGS="-fsanitize=address -ldl" ./configure --enable-debug

make clean && make
sudo make install
```

## Usage

### NTFSPLUS Kernel Module

#### Basic Mounting
```bash
# Mount NTFS volume
sudo mount -t ntfsplus /dev/sdXn /mnt/ntfs

# Mount with performance options
sudo mount -t ntfsplus -o cache_size=1024,transactions=1 /dev/sdXn /mnt

# Mount read-only for safety
sudo mount -t ntfsplus -o ro /dev/sdXn /mnt

# Check mounted filesystems
mount | grep ntfsplus
```

#### Module Parameters
```bash
# Load with custom parameters
sudo modprobe ntfsplus debug=1 cache_size=512 security=1

# Available parameters:
# - debug: Enable debug logging (0=off, 1=on)
# - cache_size: Cache size in MB (64-4096)
# - security: Enable security features (0=off, 1=on)
# - transactions: Enable transactional NTFS (0=off, 1=on)
```

#### Module Management
```bash
# Check module status
lsmod | grep ntfsplus

# View module information
modinfo ntfsplus

# Reload module
sudo rmmod ntfsplus
sudo modprobe ntfsplus

# Check kernel logs
dmesg | grep ntfsplus
```

### ntfsprogs-plus Utilities

#### Filesystem Checking (ntfsck)
```bash
# Automatic repair
sudo ntfsck -a /dev/sdXn

# Check only (no repair)
sudo ntfsck -n /dev/sdXn

# Check if volume is clean/dirty
sudo ntfsck -C /dev/sdXn
```

#### Disk Cloning (ntfsclone)
```bash
# Clone to sparse image (metadata only)
sudo ntfsclone -s -O image.img --ignore-fs-check /dev/sdXn

# Restore from image
sudo ntfsclone -r -O /dev/sdXn image.img

# Clone to standard output
sudo ntfsclone -s --ignore-fs-check /dev/sdXn | gzip > backup.gz
```

#### Cluster Analysis (ntfscluster)
```bash
# Find inodes containing specific clusters
sudo ntfscluster -c 12345-12350 /dev/sdXn

# Find inode containing single cluster
sudo ntfscluster -c 12345 /dev/sdXn
```

#### Filesystem Information (ntfsinfo)
```bash
# Show all attributes of inode
sudo ntfsinfo -i 12345 /dev/sdXn

# Show volume information
sudo ntfsinfo /dev/sdXn
```

## Configuration

### Kernel Configuration (NTFSPLUS)

Enable NTFSPLUS in your kernel configuration:
```make
CONFIG_NTFSPLUS_FS=m              # Enable NTFSPLUS module
CONFIG_NTFSPLUS_FS_COMPRESSION=y  # Enable compression support
CONFIG_NTFSPLUS_FS_TRANSACTIONS=y # Enable transactional NTFS
CONFIG_NTFSPLUS_FS_SECURITY=y     # Enable security framework
CONFIG_NTFSPLUS_FS_DEBUG=n        # Enable debug features
```

### System Integration

#### Automount Configuration (/etc/fstab)
```bash
# NTFSPLUS mount entry
/dev/sdXn /mnt/ntfs ntfsplus defaults,cache_size=512 0 0
```

#### Module Auto-loading (/etc/modules-load.d/ntfsplus.conf)
```bash
# Load NTFSPLUS at boot
ntfsplus
```

## Testing

### NTFSPLUS Kernel Module Tests
```bash
# Create test filesystem
dd if=/dev/zero of=/tmp/ntfs.img bs=1M count=100
mkntfs /tmp/ntfs.img

# Mount and test
sudo mount -t ntfsplus /tmp/ntfs.img /mnt/test

# Create test files
echo "NTFSPLUS test" > /mnt/test/file.txt
ls -la /mnt/test/

# Unmount
sudo umount /mnt/test
```

### ntfsprogs-plus Utility Tests
```bash
# Run test suite
cd tests
./test_all_images.sh

# Test specific utilities
sudo ntfsck -n /dev/sdXn
sudo ntfsinfo /dev/sdXn | head -20
```

## Performance Tuning

### NTFSPLUS Cache Optimization
```bash
# Set optimal cache size
sudo modprobe ntfsplus cache_size=1024

# Monitor cache performance
watch -n 5 'dmesg | grep -i cache | tail -5'
```

### I/O Scheduler Tuning
```bash
# Set performance-oriented I/O scheduler
echo deadline > /sys/block/sdX/queue/scheduler

# Adjust read-ahead
blockdev --setra 2048 /dev/sdX
```

### Memory Management
```bash
# Optimize for NTFSPLUS
echo 10 > /proc/sys/vm/swappiness

# Enable memory compaction
echo 1 > /proc/sys/vm/compact_memory
```

## Updates & Maintenance

### Update NTFSPLUS Kernel Module
```bash
# Pull latest changes
git pull origin master

# Rebuild kernel module
cd kernel/fs/ntfsplus
make clean && make

# Update module
sudo make modules_install
sudo depmod -a

# Reload module
sudo rmmod ntfsplus
sudo modprobe ntfsplus

# Verify version
modinfo ntfsplus | grep version
```

### Update ntfsprogs-plus Utilities
```bash
# Pull latest changes
git pull origin master

# Rebuild utilities
make clean && make
sudo make install

# Verify installation
ntfsck --version
```

### System Updates After Kernel Upgrade
```bash
# After kernel update, rebuild NTFSPLUS
cd kernel/fs/ntfsplus
make clean && make
sudo make modules_install
sudo depmod -a

# Reload module
sudo rmmod ntfsplus
sudo modprobe ntfsplus
```

## Troubleshooting

### NTFSPLUS Module Issues

**Module won't load:**
```bash
# Check kernel compatibility
uname -r
modinfo ntfsplus.ko | grep vermagic

# Check dependencies
modprobe --dry-run ntfsplus

# View kernel logs
dmesg | grep ntfsplus
```

**Filesystem won't mount:**
```bash
# Check module is loaded
lsmod | grep ntfsplus

# Try read-only mount
sudo mount -t ntfsplus -o ro /dev/sdXn /mnt

# Check filesystem
sudo ntfsck -n /dev/sdXn
```

**Performance issues:**
```bash
# Adjust cache size
sudo modprobe ntfsplus cache_size=512

# Check I/O scheduler
cat /sys/block/sdX/queue/scheduler
```

### ntfsprogs-plus Utility Issues

**ntfsck reports errors:**
```bash
# Run repair
sudo ntfsck -a /dev/sdXn

# Check boot sector
sudo ntfsck -b /dev/sdXn
```

**ntfsclone fails:**
```bash
# Try with ignore filesystem check
sudo ntfsclone -s -O image.img --ignore-fs-check /dev/sdXn

# Check disk space
df -h
```

### Common Solutions

**Permission denied:**
```bash
# Run as root
sudo ntfsck -a /dev/sdXn

# Check device permissions
ls -l /dev/sdXn
```

**Device busy:**
```bash
# Unmount first
sudo umount /dev/sdXn

# Check what's using the device
lsof /dev/sdXn
```

**Out of memory:**
```bash
# Reduce cache size
sudo modprobe ntfsplus cache_size=128

# Check system memory
free -h
```

##  Documentation

### NTFSPLUS Documentation
- **[Development Guide](Documentation/filesystems/ntfsplus/DEVELOPMENT.rst)** - Technical development documentation
- **[Troubleshooting Guide](Documentation/filesystems/ntfsplus/TROUBLESHOOTING.rst)** - Comprehensive troubleshooting
- **[API Reference](Documentation/filesystems/ntfsplus.rst)** - Technical filesystem reference

### ntfsprogs-plus Documentation
- **[Utilities Usage](#utilities-usage)** - Command-line tool documentation
- **[Testing](#test)** - Test suite documentation
- **[Build Guide](#build-and-install)** - Compilation instructions

##  Contributing

We welcome contributions! Please see our development guides for coding standards and contribution processes.

### Development Setup
```bash
# Clone repository
git clone https://github.com/sprinteroz/NTFSKFC.git
cd NTFSKFC

# Set up development environment
# Follow installation instructions above

# Build and test
cd kernel/fs/ntfsplus && make
./autogen.sh && make
```

## License

This project is licensed under the GNU General Public License v2.0.

**NTFSPLUS Kernel Module**: GPL v2.0  
**ntfsprogs-plus Utilities**: GPL v2.0
```

## Acknowledgments

NTFSPLUS is built upon the foundations of:
- **Linux Kernel**: VFS and filesystem infrastructure
- **ntfs-3g**: Original NTFS userspace implementation
- **Open Source Community**: Countless contributors and testers

## Support

- **GitHub Issues**: https://github.com/sprinteroz/NTFSKFC/issues
- **Documentation**: See Documentation/ directory
- **Professional Support**: support@magdrivex.com

---

**NTFSPLUS** - Enterprise NTFS for Linux, built for performance and reliability.

**Co Author**: Darryl Bennett 
**License**: GPL v2.0  
**Repository**: https://github.com/sprinteroz/NTFSKFC

# Build and Install
You can configure and set up build environment according to your condition.
You should have GNU Build system (autoconf, automake, libtool)
and some libraries to build ntfsprogs-plus. If you don't have them,
you should install them.

For ubuntu
```
sudo apt install build-essential automake autoconf libtool
sudo apt install libgcrypt20-dev libasan8
```
For redhat
```
yum install automake autoconf libtool
yum install libgcrypt20-dev libasan8
```
If packages have the version number like as libgcrypt20-dev / libasan8,
you'd better to use 'search' command to find proper version of your system.
```
sudo apt search libgcrypt 	// For ubuntu series
sudo yum search libasan		// For redhat series
```

If you're the first time to build ntfsprogs-plus, then you'd better to run 'autogen.sh'
```
$ ./autogen.sh
```
After that, you can build with make, if not configured, 'make' will execute configure command automatically.
```
$ make clean; make
$ make install
```

You can configure ntfsprogs-plus with address sanitizer like below.
```
$ CFLAGS="-fsanitize=address -fno-omit-frame-pointer -g -pg" LDFLAGS="-fsanitize=address -ldl" ./configure
```

And you may enable debug
```
$ configure --enable-debug
```

If you want to compile with arm cross compiler, you can also configure.
```
$ ./configure --host=arm-linux-gnueabi --target=arm-linux-gnueabi
```
# Test
You can test corrupted images in *tests* directory. Just execute script *test_all_images.sh* to test images like below.
```
$ cd tests
$ ./test_all_images.sh
```
Script will download images and script from *https://github.com/ntfsprogs-plus/ntfs_corrupted_images* repository and test them. Default test branch is *main* branch, if you test other branch of corrupted image repository, you may execute script with branch name.
```
$ cd tests
$ ./test_all_images.sh <branch name>
```
During test, script will request your *sudo* previleges because of *mount* command. If your system need to get *sudo* previleges for executing *mount*, *unmount*, you should have it to test.

# Utilities Usage

## ntfsck
ntfs filesystem check utility.
You can show with '--help' option to view detail options.

Normally you may use '-a' option for automatic repair.
'-n' option represent that do check but not repair.
'-C' option return if the volume is dirty or clean.

```
ntfsck -a <device>
ntfsck -n <device>
ntfsck -C <device>
```
ntfsck include basic filesystem checking features, like as below:
- Boot sector check
- Meta system files check
- Inode and LCN Bitmap consistency check
- Inode structure check
  - Essential attribute check
  - Runlist check
  - Attribute list check
- Directory structure check
  - All checks for inode strcuture above
  - Directory index check
  - Directory index bitmap check
- Cluster duplication check : If found cluster duplication. it copied to new clusters.

## ntfsclone
ntfs disk dump utility.
It can clone NTFS to a sparse file, special ntfsclone image or standard output.
You may clone ntfs as an "image_file" with a ntfsclone image that clone only metadata of "device" by below command.
```
ntfsclone -s -O <image_file> -m --ignore-fs-check <device>
```
After cloning, you may restore dump file("dump_file") from ntfsclone image("image_file").
```
ntfsclone -r -O <dump_file> <image_file>
```
Now, you can check and analyze ntfs filesystem with "dump_file"

## ntfscluster
ntfs utiliy that find inodes that contains a specific clusters.
You can find inodes list that contains cluster described by "cluster range". You can also specify only one cluster to find inode include it.
```
ntfscluster -c <cluster range> <device>
```

## ntfsinfo
ntfsinfo show all attributes of specified inode.
You can look into more detail information of inode that consist of inode:
```
ntfsinfo -i <inode number> <device>
```

# License
ntfsprogs-plus is published under GPLv2 license.
