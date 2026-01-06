.. SPDX-License-Identifier: GPL-2.0

.\" Author: Darryl Bennett
.\" Created: January 6, 2026

===============================
NTFSPLUS Troubleshooting Guide
===============================

This comprehensive troubleshooting guide covers common issues, debugging techniques,
and solutions for NTFSPLUS filesystem problems. Whether you're a developer or
end-user, this guide will help you diagnose and resolve issues.

Quick Diagnosis
===============

System Information
------------------

Before troubleshooting, collect essential system information:

.. code-block:: bash

   # Kernel version and architecture
   uname -a

   # NTFSPLUS module information
   modinfo ntfsplus

   # Kernel logs (NTFSPLUS specific)
   dmesg | grep -i ntfsplus

   # Loaded modules
   lsmod | grep ntfsplus

   # Mount information
   mount | grep ntfsplus

   # Disk information
   lsblk -f
   blkid /dev/sdXn

Module Loading Issues
=====================

Module Won't Load
-----------------

**Symptoms:**
- ``modprobe ntfsplus`` fails
- ``insmod ntfsplus.ko`` returns error
- Kernel logs show dependency or symbol errors

**Diagnosis:**

.. code-block:: bash

   # Check kernel version compatibility
   uname -r
   modinfo ntfsplus.ko | grep vermagic

   # Check for missing dependencies
   modprobe --dry-run ntfsplus

   # Check kernel logs for errors
   dmesg | tail -20

**Common Solutions:**

1. **Kernel Version Mismatch:**

   .. code-block:: bash

      # Rebuild module for current kernel
      cd /path/to/ntfsplus/source
      make clean && make
      sudo make modules_install
      sudo depmod -a

2. **Missing Kernel Headers:**

   .. code-block:: bash

      # Install matching kernel headers
      sudo apt-get install linux-headers-$(uname -r)

3. **Dependency Issues:**

   .. code-block:: bash

      # Check module dependencies
      grep depends /lib/modules/$(uname -r)/modules.dep | grep ntfsplus

      # Load dependencies first
      sudo modprobe dependency_module

4. **Kernel Configuration Issues:**

   .. code-block:: bash

      # Check if required kernel features are enabled
      grep CONFIG_NTFSPLUS /boot/config-$(uname -r)
      grep CONFIG_VFS /boot/config-$(uname -r)

Module Parameters Not Working
-----------------------------

**Symptoms:**
- Module loads but ignores parameters
- ``dmesg`` shows "unknown parameter" warnings

**Diagnosis:**

.. code-block:: bash

   # Check current module parameters
   systool -v -m ntfsplus

   # Try loading with parameters
   sudo modprobe ntfsplus debug=1 cache_size=512

   # Check if parameters were accepted
   cat /sys/module/ntfsplus/parameters/debug

**Solutions:**

1. **Parameter Syntax Error:**

   .. code-block:: bash

      # Correct syntax
      sudo insmod ntfsplus.ko debug=1 cache_size=512

      # Not this (wrong)
      sudo insmod ntfsplus.ko debug=1,cache_size=512

2. **Parameter Range Issues:**

   .. code-block:: bash

      # Check valid parameter ranges
      # debug: 0-1
      # cache_size: 64-4096 (MB)
      # security: 0-1
      # transactions: 0-1

Mount Issues
============

Filesystem Won't Mount
----------------------

**Symptoms:**
- ``mount -t ntfsplus /dev/sdXn /mnt`` fails
- Error messages like "unknown filesystem type" or "mount failed"

**Diagnosis:**

.. code-block:: bash

   # Check if module is loaded
   lsmod | grep ntfsplus

   # Check filesystem type recognition
   blkid /dev/sdXn

   # Try mount with debug output
   sudo mount -t ntfsplus -v /dev/sdXn /mnt 2>&1

   # Check kernel logs
   dmesg | grep -A 5 -B 5 ntfsplus

**Common Solutions:**

1. **Module Not Loaded:**

   .. code-block:: bash

      sudo modprobe ntfsplus

2. **Filesystem Not Recognized:**

   .. code-block:: bash

      # Check if it's actually NTFS
      file -s /dev/sdXn

      # Try alternative mount options
      sudo mount -t ntfsplus -o ro /dev/sdXn /mnt

3. **Device Busy or In Use:**

   .. code-block:: bash

      # Check if device is mounted elsewhere
      mount | grep /dev/sdXn

      # Force unmount if needed
      sudo umount /dev/sdXn

4. **Permission Issues:**

   .. code-block:: bash

      # Check device permissions
      ls -l /dev/sdXn

      # Mount as root
      sudo mount -t ntfsplus /dev/sdXn /mnt

Mount Hangs or Times Out
------------------------

**Symptoms:**
- Mount command hangs indefinitely
- System becomes unresponsive during mount
- Mount times out after long period

**Diagnosis:**

.. code-block:: bash

   # Check for I/O errors
   dmesg | grep -i "i/o error"

   # Check disk health
   sudo smartctl -H /dev/sdX

   # Check for filesystem corruption
   sudo ntfsfix /dev/sdXn

   # Try read-only mount first
   sudo mount -t ntfsplus -o ro /dev/sdXn /mnt

**Solutions:**

1. **Filesystem Corruption:**

   .. code-block:: bash

      # Run NTFS repair (Windows tool recommended)
      # Or use ntfsfix (read-only repair)
      sudo ntfsfix --no-action /dev/sdXn

2. **Hardware Issues:**

   .. code-block:: bash

      # Check disk connections
      lsblk -t

      # Run disk diagnostics
      sudo badblocks /dev/sdXn

3. **Resource Exhaustion:**

   .. code-block:: bash

      # Check memory usage
      free -h

      # Check system load
      uptime

      # Reduce cache size
      sudo modprobe ntfsplus cache_size=128

Permission Issues
-----------------

**Symptoms:**
- Access denied errors
- Cannot read/write files
- Permission denied on directories

**Diagnosis:**

.. code-block:: bash

   # Check mount options
   mount | grep ntfsplus

   # Check file permissions
   ls -l /mnt/file

   # Check mount point permissions
   ls -ld /mnt

   # Check NTFS permissions (if available)
   # Note: NTFS permissions may not be fully supported

**Solutions:**

1. **Mount Options:**

   .. code-block:: bash

      # Mount with different permissions
      sudo mount -t ntfsplus -o uid=$(id -u),gid=$(id -g) /dev/sdXn /mnt

      # Force read-write
      sudo mount -t ntfsplus -o rw,force /dev/sdXn /mnt

2. **SELinux/AppArmor Issues:**

   .. code-block:: bash

      # Check SELinux status
      sestatus

      # Temporarily disable SELinux
      sudo setenforce 0

      # Check AppArmor profiles
      sudo apparmor_status | grep ntfsplus

Performance Issues
==================

Slow File Operations
--------------------

**Symptoms:**
- File reads/writes are slow
- Directory listings take long time
- High CPU usage during I/O

**Diagnosis:**

.. code-block:: bash

   # Check cache performance
   cat /proc/ntfsplus/cache_stats 2>/dev/null || echo "No cache stats available"

   # Monitor I/O performance
   iostat -x 1

   # Check memory usage
   free -h

   # Profile with perf
   sudo perf record -a -g -- sleep 10
   sudo perf report

**Solutions:**

1. **Cache Configuration:**

   .. code-block:: bash

      # Increase cache size
      sudo rmmod ntfsplus
      sudo modprobe ntfsplus cache_size=1024

      # Monitor cache hit rate
      watch -n 5 cat /proc/ntfsplus/cache_stats

2. **I/O Scheduler:**

   .. code-block:: bash

      # Check current scheduler
      cat /sys/block/sdX/queue/scheduler

      # Change to performance-oriented scheduler
      echo deadline > /sys/block/sdX/queue/scheduler

3. **Memory Issues:**

   .. code-block:: bash

      # Check for memory pressure
      vmstat 1

      # Clear page cache
      echo 3 > /proc/sys/vm/drop_caches

High Memory Usage
-----------------

**Symptoms:**
- System runs out of memory
- NTFSPLUS uses excessive RAM
- System becomes unresponsive

**Diagnosis:**

.. code-block:: bash

   # Check NTFSPLUS memory usage
   ps aux | grep ntfsplus

   # Monitor memory usage
   htop  # or top

   # Check cache size
   cat /sys/module/ntfsplus/parameters/cache_size

   # Check for memory leaks
   sudo dmesg | grep -i leak

**Solutions:**

1. **Reduce Cache Size:**

   .. code-block:: bash

      # Unload and reload with smaller cache
      sudo rmmod ntfsplus
      sudo modprobe ntfsplus cache_size=256

2. **Memory Leak Investigation:**

   .. code-block:: bash

      # Enable kmemleak (if compiled in kernel)
      echo scan > /sys/kernel/debug/kmemleak

      # Check for leaks
      cat /sys/kernel/debug/kmemleak

3. **System Memory Tuning:**

   .. code-block:: bash

      # Adjust swappiness
      echo 10 > /proc/sys/vm/swappiness

      # Enable memory compaction
      echo 1 > /proc/sys/vm/compact_memory

File Access Issues
==================

Cannot Read Files
-----------------

**Symptoms:**
- File read operations fail
- "Input/output error" messages
- Files appear corrupted

**Diagnosis:**

.. code-block:: bash

   # Check file accessibility
   sudo file /mnt/path/to/file

   # Try reading with different tools
   sudo cat /mnt/path/to/file | head -10

   # Check kernel logs
   dmesg | grep -A 5 -B 5 "read.*error"

   # Test with different mount options
   sudo mount -t ntfsplus -o ro /dev/sdXn /mnt

**Solutions:**

1. **Filesystem Corruption:**

   .. code-block:: bash

      # Check filesystem health
      sudo ntfsfix --no-action /dev/sdXn

      # Try repair (use with caution)
      sudo ntfsfix /dev/sdXn

2. **Compression Issues:**

   .. code-block:: bash

      # Disable compression support
      sudo rmmod ntfsplus
      sudo modprobe ntfsplus compression=0

3. **Attribute Access Problems:**

   .. code-block:: bash

      # Enable debug logging
      sudo rmmod ntfsplus
      sudo modprobe ntfsplus debug=1

Cannot Write Files
------------------

**Symptoms:**
- File write operations fail
- "Read-only file system" errors
- Permission denied on write

**Diagnosis:**

.. code-block:: bash

   # Check mount options
   mount | grep ntfsplus

   # Check filesystem flags
   sudo ntfsinfo /dev/sdXn | grep flags

   # Try write operation
   echo "test" > /mnt/test.txt 2>&1

   # Check kernel logs
   dmesg | grep -i write

**Solutions:**

1. **Read-Only Mount:**

   .. code-block:: bash

      # Remount as read-write
      sudo mount -t ntfsplus -o remount,rw /dev/sdXn

2. **Filesystem Issues:**

   .. code-block:: bash

      # Check if filesystem is dirty
      sudo ntfsfix --clear-dirty /dev/sdXn

3. **Permission Issues:**

   .. code-block:: bash

      # Mount with proper permissions
      sudo mount -t ntfsplus -o uid=1000,gid=1000 /dev/sdXn /mnt

Directory Issues
----------------

Cannot List Directories
-----------------------

**Symptoms:**
- ``ls`` commands fail in directories
- "Permission denied" on directory access
- Directory appears empty

**Diagnosis:**

.. code-block:: bash

   # Check directory permissions
   ls -ld /mnt/directory

   # Try listing as root
   sudo ls -la /mnt/directory

   # Check kernel logs
   dmesg | grep -i "readdir\|lookup"

   # Test with debug logging
   sudo rmmod ntfsplus
   sudo modprobe ntfsplus debug=1

**Solutions:**

1. **Index Corruption:**

   .. code-block:: bash

      # Check directory index
      sudo ntfsfix --index-check /dev/sdXn

2. **Permission Issues:**

   .. code-block:: bash

      # Mount with proper permissions
      sudo mount -t ntfsplus -o dmask=022,fmask=133 /dev/sdXn /mnt

3. **Directory Structure Issues:**

   .. code-block:: bash

      # Enable transaction logging
      sudo rmmod ntfsplus
      sudo modprobe ntfsplus transactions=1

Advanced Debugging
==================

Kernel Debugging
-----------------

**Enabling Debug Features:**

.. code-block:: bash

   # Load with debug options
   sudo modprobe ntfsplus debug=1

   # Check debug output
   dmesg | grep NTFSPLUS

   # Increase log level
   echo 8 > /proc/sys/kernel/printk

**Using ftrace:**

.. code-block:: bash

   # Enable function tracing
   echo 1 > /sys/kernel/debug/tracing/tracing_on
   echo ntfsplus_* > /sys/kernel/debug/tracing/set_ftrace_filter
   echo function > /sys/kernel/debug/tracing/current_tracer

   # View trace output
   cat /sys/kernel/debug/tracing/trace

Memory Debugging
----------------

**KASAN (Kernel Address Sanitizer):**

.. code-block:: bash

   # Check if KASAN is enabled
   grep KASAN /boot/config-$(uname -r)

   # Look for memory issues
   dmesg | grep kasan

**kmemleak:**

.. code-block:: bash

   # Enable kmemleak
   echo scan > /sys/kernel/debug/kmemleak

   # Check for leaks
   cat /sys/kernel/debug/kmemleak

Performance Profiling
---------------------

**Using perf:**

.. code-block:: bash

   # Profile NTFSPLUS operations
   sudo perf record -a -g --pid=$(pidof ntfsplus_test)

   # Analyze results
   sudo perf report

**Using SystemTap:**

.. code-block:: bash

   # Monitor NTFSPLUS function calls
   stap -e 'probe kernel.function("ntfsplus_*") { println(ppfunc()) }'

Crash Analysis
--------------

**Kernel Oops Analysis:**

.. code-block:: bash

   # Check kernel logs for oops
   dmesg | grep -A 20 -B 5 "Oops\|BUG"

   # Decode stack trace
   addr2line -e /path/to/ntfsplus.ko <address>

**Core Dump Analysis:**

.. code-block:: bash

   # Enable kdump
   sudo systemctl enable kdump

   # Analyze vmcore
   crash /usr/lib/debug/lib/modules/$(uname -r)/vmlinux vmcore

Data Recovery
=============

Filesystem Corruption
---------------------

**Symptoms:**
- Filesystem mount fails
- Files appear corrupted
- I/O errors reported

**Recovery Steps:**

1. **Read-Only Mount:**

   .. code-block:: bash

      # Try read-only mount first
      sudo mount -t ntfsplus -o ro /dev/sdXn /mnt

2. **Filesystem Check:**

   .. code-block:: bash

      # Use Windows chkdsk (recommended)
      # Or ntfsfix (Linux alternative)
      sudo ntfsfix /dev/sdXn

3. **Data Recovery:**

   .. code-block:: bash

      # Use testdisk for partition recovery
      sudo testdisk /dev/sdX

      # Use photorec for file recovery
      sudo photorec /dev/sdX

4. **Backup Important Data:**

   .. code-block:: bash

      # If mount works, backup data
      sudo cp -r /mnt /backup/location

Lost Files/Directories
---------------------

**Recovery Techniques:**

.. code-block:: bash

   # Check for deleted files
   sudo ntfsundelete /dev/sdXn

   # Use scalpel for carving
   sudo scalpel -c /etc/scalpel/scalpel.conf /dev/sdXn -o /recovery

   # Try ntfsprogs utilities
   sudo ntfsls /dev/sdXn

Getting Help
============

Support Resources
-----------------

**Community Support:**

- **GitHub Issues**: https://github.com/darrylbennett/NTFSPLUS/issues
- **Mailing List**: ntfsplus-devel@lists.magdrivex.com
- **Documentation**: https://ntfsplus.readthedocs.io/

**Professional Support:**

- **Enterprise Support**: support@magdrivex.com
- **Consulting Services**: consulting@magdrivex.com

Bug Report Template
-------------------

When reporting bugs, include:

.. code-block:: text

   NTFSPLUS Bug Report
   ===================

   System Information:
   - Kernel version: $(uname -a)
   - NTFSPLUS version: $(modinfo ntfsplus | grep version)
   - Distribution: $(lsb_release -a 2>/dev/null || cat /etc/os-release)

   Problem Description:
   - What were you trying to do?
   - What happened instead?
   - Expected behavior?

   Steps to Reproduce:
   1. Step 1
   2. Step 2
   3. Step 3

   Error Messages:
   $(dmesg | grep -i ntfsplus)

   Filesystem Information:
   $(ntfsinfo /dev/sdXn 2>/dev/null || echo "ntfsinfo not available")

   Mount Information:
   $(mount | grep ntfsplus)

   Module Parameters:
   $(systool -v -m ntfsplus 2>/dev/null || echo "systool not available")

   Additional Context:
   - Hardware information
   - Recent system changes
   - Any relevant logs

Preventive Maintenance
======================

Regular Maintenance Tasks
-------------------------

**Weekly Tasks:**

.. code-block:: bash

   # Check filesystem health
   sudo ntfsfix --no-action /dev/sdXn

   # Monitor cache statistics
   cat /proc/ntfsplus/cache_stats

   # Check kernel logs
   dmesg | grep -i ntfsplus

**Monthly Tasks:**

.. code-block:: bash

   # Full filesystem check
   sudo ntfsfix /dev/sdXn

   # Update NTFSPLUS module
   sudo modprobe -r ntfsplus
   sudo modprobe ntfsplus

**Performance Monitoring:**

.. code-block:: bash

   # Monitor I/O performance
   iostat -x 1

   # Check memory usage
   free -h

   # Monitor system load
   uptime

Backup Strategies
-----------------

**Filesystem Backup:**

.. code-block:: bash

   # Create filesystem image
   sudo dd if=/dev/sdXn of=/backup/ntfs_backup.img bs=4M

   # Use ntfsclone for compressed backup
   sudo ntfsclone -s -o /dev/sdXn /backup/ntfs_compressed.img

**Data Backup:**

.. code-block:: bash

   # Backup important files
   sudo rsync -av /mnt/data /backup/data

   # Use tar for archiving
   sudo tar czf /backup/data.tar.gz /mnt/data

Configuration Best Practices
----------------------------

**Optimal Module Parameters:**

.. code-block:: bash

   # Performance-oriented
   sudo modprobe ntfsplus \
       cache_size=1024 \
       transactions=1 \
       security=1 \
       debug=0

**Filesystem Tuning:**

.. code-block:: bash

   # Mount options for performance
   sudo mount -t ntfsplus \
       -o cache_size=1024,transactions=1 \
       /dev/sdXn /mnt

**System Tuning:**

.. code-block:: bash

   # I/O scheduler
   echo deadline > /sys/block/sdX/queue/scheduler

   # Memory management
   echo 10 > /proc/sys/vm/swappiness

   # Filesystem limits
   echo 1048576 > /proc/sys/fs/file-max

References
==========

- `NTFSPLUS Documentation <https://ntfsplus.readthedocs.io/>`_
- `Linux Kernel Documentation <https://www.kernel.org/doc/>`_
- `NTFS-3G Documentation <https://www.tuxera.com/community/ntfs-3g-manual/>`_
- `Filesystem Troubleshooting <https://www.kernel.org/doc/html/latest/filesystems/index.html>`_
