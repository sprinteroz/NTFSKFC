.. SPDX-License-Identifier: GPL-2.0

.\" Author: Darryl Bennett
.\" Created: January 6, 2026

===============================
NTFSPLUS Development Guide
===============================

This document provides comprehensive information for developers working on the
NTFSPLUS kernel filesystem implementation. It covers architecture, APIs,
debugging techniques, and contribution guidelines.

Architecture Overview
=====================

NTFSPLUS is a complete NTFS filesystem implementation for the Linux kernel that
provides superior performance and enterprise-grade features compared to Windows
NTFS. The implementation is structured as follows:

Core Components
---------------

VFS Interface Layer
^^^^^^^^^^^^^^^^^^^

The VFS (Virtual File System) interface layer provides the standard Linux
filesystem operations:

- **Mount/Unmount**: ``ntfsplus_mount()``, ``ntfsplus_kill_sb()``
- **Superblock Operations**: ``ntfsplus_fill_super()``
- **Inode Operations**: File/directory operations
- **File Operations**: Read/write/seek operations

NTFS On-Disk Structures
^^^^^^^^^^^^^^^^^^^^^^^

NTFSPLUS implements full NTFS on-disk format support:

- **Boot Sector**: Volume geometry and MFT location
- **Master File Table (MFT)**: File metadata storage
- **Attributes**: Data streams, filenames, security descriptors
- **Runlists**: Cluster mapping for non-resident attributes
- **Index Structures**: Directory organization (B+ trees)

Enterprise Features
^^^^^^^^^^^^^^^^^^^

Advanced features for enterprise deployments:

- **Transactional NTFS**: ACID-compliant operations
- **Intelligent Caching**: Multi-level LRU cache system
- **Compression**: LZNT1 algorithm support
- **Security Framework**: SELinux integration
- **Performance Monitoring**: Comprehensive statistics

API Reference
=============

Core Functions
--------------

Mount Operations
^^^^^^^^^^^^^^^^

.. kernel-doc:: fs/ntfsplus/ntfsplus_main.c
   :functions: ntfsplus_mount ntfsplus_kill_sb ntfsplus_fill_super

File Operations
^^^^^^^^^^^^^^^

.. kernel-doc:: fs/ntfsplus/ntfsplus_main.c
   :functions: ntfsplus_file_open ntfsplus_file_release ntfsplus_file_read_iter ntfsplus_file_write_iter

Directory Operations
^^^^^^^^^^^^^^^^^^^^

.. kernel-doc:: fs/ntfsplus/ntfsplus_main.c
   :functions: ntfsplus_lookup ntfsplus_create ntfsplus_readdir

Volume Management
^^^^^^^^^^^^^^^^^

.. kernel-doc:: fs/ntfsplus/kernel_volume.c
   :functions: ntfsplus_volume_alloc ntfsplus_volume_startup ntfsplus_volume_shutdown

Caching System
^^^^^^^^^^^^^^

.. kernel-doc:: fs/ntfsplus/kernel_cache.c
   :functions: ntfsplus_cache_init ntfsplus_cache_lookup ntfsplus_cache_insert ntfsplus_cache_invalidate

Transactional NTFS
^^^^^^^^^^^^^^^^^^

.. kernel-doc:: fs/ntfsplus/kernel_transaction.c
   :functions: ntfsplus_transaction_begin ntfsplus_transaction_commit ntfsplus_transaction_rollback

Compression Engine
^^^^^^^^^^^^^^^^^^

.. kernel-doc:: fs/ntfsplus/kernel_compression.c
   :functions: ntfsplus_lznt1_compress ntfsplus_lznt1_decompress ntfsplus_compress_chunk

Security Framework
^^^^^^^^^^^^^^^^^^

.. kernel-doc:: fs/ntfsplus/kernel_security.c
   :functions: ntfsplus_security_init ntfsplus_get_security_context ntfsplus_set_security_context

Attribute Handling
^^^^^^^^^^^^^^^^^^

.. kernel-doc:: fs/ntfsplus/kernel_attrib.c
   :functions: ntfsplus_attr_alloc ntfsplus_attr_open ntfsplus_attr_pread ntfsplus_attr_pwrite

Runlist Management
^^^^^^^^^^^^^^^^^^

.. kernel-doc:: fs/ntfsplus/kernel_runlist.c
   :functions: ntfs_rl_vcn_to_lcn ntfs_mapping_pairs_decompress ntfs_rl_pread ntfs_rl_pwrite

MFT Operations
^^^^^^^^^^^^^^

.. kernel-doc:: fs/ntfsplus/kernel_mft.c
   :functions: ntfs_mft_records_read ntfs_mft_records_write ntfs_mft_record_alloc ntfs_mft_record_free

Inode Management
^^^^^^^^^^^^^^^^

.. kernel-doc:: fs/ntfsplus/kernel_inode.c
   :functions: ntfsplus_inode_allocate ntfsplus_inode_open ntfsplus_inode_close

Utility Functions
^^^^^^^^^^^^^^^^^

.. kernel-doc:: fs/ntfsplus/kernel_utils.c
   :functions: ntfsplus_malloc ntfsplus_free ntfsplus_ucstombs ntfsplus_ftime

Logging System
^^^^^^^^^^^^^^

.. kernel-doc:: fs/ntfsplus/kernel_logging.c
   :functions: ntfsplus_log_error ntfsplus_log_info ntfsplus_log_debug ntfsplus_set_log_level

Data Structures
===============

NTFSPLUS Volume Structure
-------------------------

The ``ntfsplus_volume`` structure represents an NTFS volume:

.. code-block:: c

   struct ntfsplus_volume {
       struct super_block *sb;        /* VFS superblock */
       struct block_device *bdev;     /* Block device */
       u32 cluster_size;              /* Cluster size in bytes */
       u64 nr_clusters;               /* Number of clusters */
       u32 mft_record_size;           /* MFT record size */
       u64 mft_lcn;                   /* LCN of MFT */
       u64 mftmirr_lcn;               /* LCN of MFT mirror */
       unsigned long flags;           /* Volume flags */
       u8 major_ver, minor_ver;       /* NTFS version */
       struct ntfsplus_attr *mft_na;  /* MFT data attribute */
       struct kmem_cache *mft_cache;  /* MFT record cache */
       ntfschar *upcase;              /* Upcase table */
       struct mutex volume_mutex;     /* Volume synchronization */
   };

NTFSPLUS Inode Structure
------------------------

The ``ntfsplus_inode`` structure extends VFS inodes:

.. code-block:: c

   struct ntfsplus_inode {
       u64 mft_no;                    /* MFT record number */
       struct ntfsplus_volume *vol;   /* Parent volume */
       MFT_RECORD *mrec;             /* MFT record */
       unsigned long flags;          /* Inode flags */
       s64 data_size;                /* Data size */
       s64 allocated_size;           /* Allocated size */
       struct mutex lock;            /* Inode synchronization */
       sle64 creation_time;          /* File timestamps */
       sle64 last_data_change_time;
       sle64 last_mft_change_time;
       sle64 last_access_time;
       int nr_extents;               /* Extent information */
       struct ntfsplus_inode **extent_nis;
       struct ntfsplus_inode *base_ni;
   };

Cache System Architecture
-------------------------

The NTFSPLUS caching system uses a multi-level design:

.. code-block:: c

   struct ntfsplus_cache {
       struct rb_root root;           /* Red-black tree for lookups */
       struct list_head lru_list;     /* LRU eviction list */
       spinlock_t lock;               /* Concurrency protection */
       size_t max_size;               /* Maximum cache size */
       size_t current_size;           /* Current size */
       struct ntfsplus_cache_stats stats; /* Performance statistics */
       struct workqueue_struct *workqueue; /* Background operations */
   };

   struct ntfsplus_cache_entry {
       struct rb_node node;           /* Tree node */
       u64 key;                       /* Cache key (inode + offset) */
       void *data;                    /* Cached data */
       size_t size;                   /* Data size */
       u32 state;                     /* Entry state */
       atomic_t refcount;             /* Reference counting */
       struct list_head lru_list;     /* LRU list entry */
   };

Build System
============

Kernel Configuration
--------------------

NTFSPLUS supports the following Kconfig options:

.. code-block:: make

   CONFIG_NTFSPLUS_FS=m              # Enable NTFSPLUS module
   CONFIG_NTFSPLUS_FS_COMPRESSION=y  # Enable compression support
   CONFIG_NTFSPLUS_FS_TRANSACTIONS=y # Enable transactional NTFS
   CONFIG_NTFSPLUS_FS_SECURITY=y     # Enable security framework
   CONFIG_NTFSPLUS_FS_DEBUG=n        # Enable debug features
   CONFIG_NTFSPLUS_FS_STATS=y        # Enable statistics collection

Build Process
-------------

To build NTFSPLUS:

.. code-block:: bash

   # Navigate to kernel source
   cd /usr/src/linux

   # Configure kernel (enable NTFSPLUS)
   make menuconfig  # Select File systems -> NTFSPLUS

   # Build the module
   make fs/ntfsplus/ntfsplus.ko

   # Install the module
   sudo make modules_install
   sudo depmod -a

   # Load the module
   sudo modprobe ntfsplus

Cross-Compilation
-----------------

For cross-compilation to different architectures:

.. code-block:: bash

   # Set cross-compilation environment
   export ARCH=arm64
   export CROSS_COMPILE=aarch64-linux-gnu-

   # Configure for target architecture
   make defconfig  # Or use saved config

   # Build NTFSPLUS module
   make fs/ntfsplus/ntfsplus.ko

Module Parameters
-----------------

Runtime configuration parameters:

.. code-block:: c

   debug=1              /* Enable debug logging */
   cache_size=512       /* Cache size in MB */
   security=1           /* Enable security features */
   transactions=1       /* Enable transactional NTFS */

Testing Framework
=================

Unit Tests
----------

NTFSPLUS includes comprehensive unit tests:

.. code-block:: bash

   # Run cache system tests
   ./test_ntfsplus_cache

   # Run compression tests
   ./test_ntfsplus_compression

   # Run transaction tests
   ./test_ntfsplus_transactions

Integration Tests
-----------------

Full filesystem integration tests:

.. code-block:: bash

   # Create test filesystem
   dd if=/dev/zero of=/tmp/ntfs.img bs=1M count=100
   mkntfs /tmp/ntfs.img

   # Mount and test operations
   sudo mount -t ntfsplus /tmp/ntfs.img /mnt/test

   # Run filesystem tests
   ./test_ntfsplus_integration

Stress Testing
--------------

Performance and stress testing:

.. code-block:: bash

   # Multi-threaded file operations
   ./stress_test --threads=16 --duration=3600

   # Large file operations
   ./large_file_test --size=1G

   # Mount/unmount cycles
   ./mount_cycle_test --cycles=1000

Debugging Techniques
====================

Debug Logging
-------------

NTFSPLUS provides extensive debug logging:

.. code-block:: c

   // Set debug level
   ntfsplus_set_log_level(NTFSPLUS_LOG_LEVEL_DEBUG);

   // Debug macros
   ntfsplus_log_debug("Operation %s completed", op_name);
   ntfsplus_log_error("Error %d in %s", err, function);

Kernel Debuggers
-----------------

Using kernel debuggers for NTFSPLUS:

**KGDB (Kernel GNU Debugger):**

.. code-block:: bash

   # Enable KGDB in kernel config
   CONFIG_KGDB=y
   CONFIG_KGDB_SERIAL_CONSOLE=y

   # Connect debugger
   gdb vmlinux
   (gdb) target remote /dev/ttyS0

**KDB (Kernel Debugger):**

.. code-block:: bash

   # Enable KDB
   CONFIG_KDB=y

   # Enter debugger (from console)
   echo "g" > /proc/sysrq-trigger  # Break into debugger

Memory Debugging
----------------

Memory leak detection:

.. code-block:: bash

   # Enable KASAN (Kernel Address Sanitizer)
   CONFIG_KASAN=y
   CONFIG_KASAN_INLINE=y

   # Check for memory issues
   dmesg | grep kasan

   # Use kmemleak
   echo scan > /sys/kernel/debug/kmemleak
   cat /sys/kernel/debug/kmemleak

Performance Profiling
---------------------

Performance analysis tools:

.. code-block:: bash

   # Use perf for profiling
   perf record -a -g -- sleep 10
   perf report

   # Trace filesystem operations
   trace-cmd record -e 'fs:*' -e 'vfs:*'
   trace-cmd report

Common Issues & Fixes
=====================

Build Errors
------------

**Missing kernel headers:**

.. code-block:: bash

   # Install kernel headers
   sudo apt-get install linux-headers-$(uname -r)

**Configuration issues:**

.. code-block:: bash

   # Check Kconfig dependencies
   make menuconfig  # Ensure NTFSPLUS dependencies are enabled

Runtime Issues
--------------

**Module won't load:**

.. code-block:: bash

   # Check kernel version compatibility
   uname -r
   modinfo ntfsplus.ko | grep vermagic

   # Check for symbol dependencies
   modprobe --dry-run ntfsplus

**Filesystem corruption:**

.. code-block:: bash

   # Run filesystem check (read-only)
   ntfsfix /dev/sdXn

   # Enable debug logging
   insmod ntfsplus.ko debug=1

Performance Issues
------------------

**Slow file operations:**

.. code-block:: bash

   # Check cache configuration
   cat /proc/ntfsplus/stats

   # Adjust cache size
   insmod ntfsplus.ko cache_size=1024

**High memory usage:**

.. code-block:: bash

   # Monitor cache statistics
   watch -n 1 cat /proc/ntfsplus/cache_stats

   # Reduce cache size
   rmmod ntfsplus
   insmod ntfsplus.ko cache_size=256

Contribution Guidelines
=======================

Code Style
----------

NTFSPLUS follows Linux kernel coding standards:

.. code-block:: c

   /* Function documentation */
   /**
    * function_name - Brief description
    * @param1: Parameter description
    * @param2: Parameter description
    *
    * Detailed description of function purpose.
    *
    * Context: Process/Interrupt context
    * Return: Return value description
    */
   int function_name(type param1, type param2)
   {
           /* Use tabs for indentation */
           if (condition) {
                   /* Code block */
                   return 0;
           }

           return -error_code;
   }

Submitting Patches
------------------

Patch submission process:

1. **Fork the repository** on GitHub
2. **Create a feature branch** for your changes
3. **Test your changes** thoroughly
4. **Run the test suite** to ensure no regressions
5. **Submit a pull request** with detailed description

Patch Requirements
^^^^^^^^^^^^^^^^^^

- **Signed-off-by**: All patches must be signed
- **Tested-by**: Include test results
- **Reviewed-by**: For non-trivial changes
- **Fixes**: Reference bug reports or issues

Code Review Process
^^^^^^^^^^^^^^^^^^^

1. **Automated testing** runs on all patches
2. **Code style checks** ensure consistency
3. **Security review** for sensitive changes
4. **Performance impact** assessment
5. **Documentation updates** required

Testing Requirements
^^^^^^^^^^^^^^^^^^^^

All patches must pass:

- **Unit tests** for modified functions
- **Integration tests** for new features
- **Regression tests** for existing functionality
- **Performance tests** for performance-critical code
- **Stress tests** for stability verification

Release Process
===============

Version Numbering
-----------------

NTFSPLUS uses semantic versioning:

- **Major**: Breaking changes, API modifications
- **Minor**: New features, backward compatible
- **Patch**: Bug fixes, security updates

Release Checklist
-----------------

**Pre-release:**

- [ ] All tests pass
- [ ] Documentation updated
- [ ] Security audit completed
- [ ] Performance benchmarks run
- [ ] Compatibility testing done

**Release:**

- [ ] Tag created in git
- [ ] Release notes written
- [ ] Binary packages built
- [ ] Distribution repositories updated
- [ ] User notification sent

**Post-release:**

- [ ] Bug reports monitored
- [ ] Performance metrics collected
- [ ] User feedback reviewed
- [ ] Next release planning

Support & Community
===================

Mailing Lists
-------------

- **ntfsplus-devel@lists.magdrivex.com**: Development discussions
- **linux-fsdevel@vger.kernel.org**: Upstream filesystem discussions

Bug Reports
-----------

Bug reports should include:

- **Kernel version**: ``uname -a``
- **NTFSPLUS version**: ``modinfo ntfsplus``
- **Filesystem details**: ``ntfsinfo /dev/sdXn``
- **Error messages**: ``dmesg | grep ntfsplus``
- **Reproduction steps**: Detailed instructions

Documentation Updates
---------------------

**When to update documentation:**

- New features added
- API changes made
- Bug fixes that affect behavior
- Security issues discovered
- Performance improvements

**Documentation format:**

- Use reStructuredText (.rst) for technical docs
- Use KernelDoc format for API documentation
- Include code examples and usage scenarios
- Provide troubleshooting guides for common issues

References
==========

- `Linux Kernel Documentation <https://www.kernel.org/doc/>`_
- `NTFS On-Disk Format Specification <https://github.com/libyal/libfsntfs/blob/main/documentation/NTFS%20file%20system%20format.asciidoc>`_
- `Filesystem Development Guide <https://www.kernel.org/doc/html/latest/filesystems/index.html>`_
- `Kernel API Documentation <https://www.kernel.org/doc/html/latest/core-api/index.html>`_
