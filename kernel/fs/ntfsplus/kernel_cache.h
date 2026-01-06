/*
 * NTFSPLUS Kernel Module - Advanced Caching Header
 * GPL Compliant - Kernel Space Adaptation
 */

#ifndef _KERNEL_NTFS_CACHE_H
#define _KERNEL_NTFS_CACHE_H

#include "kernel_types.h"

/* Cache entry states */
#define CACHE_ENTRY_CLEAN      0x01
#define CACHE_ENTRY_DIRTY      0x02
#define CACHE_ENTRY_LOADING    0x04
#define CACHE_ENTRY_EVICTING   0x08

/* Cache types */
#define CACHE_TYPE_METADATA    0x01
#define CACHE_TYPE_DATA        0x02
#define CACHE_TYPE_DIRECTORY   0x04
#define CACHE_TYPE_ATTRIBUTE   0x08

/* Cache statistics structure */
struct ntfsplus_cache_stats {
    atomic64_t hits;
    atomic64_t misses;
    atomic64_t evictions;
    atomic64_t insertions;
    size_t size_current;
    atomic64_t size_peak;
};

/* Forward declarations */
struct ntfsplus_cache_entry;
struct ntfsplus_cache;

/* Function prototypes */
int ntfsplus_cache_init(struct ntfsplus_volume *vol, size_t max_size);
void ntfsplus_cache_exit(void);

struct ntfsplus_cache_entry *ntfsplus_cache_lookup(u64 key);
int ntfsplus_cache_insert(u64 key, void *data, size_t size, u32 type);

int ntfsplus_cache_get_stats(struct ntfsplus_cache_stats *stats);
int ntfsplus_cache_hit_ratio(void);

int ntfsplus_cache_prefetch(u64 key, u32 type);
int ntfsplus_cache_invalidate(u64 key);

#endif /* _KERNEL_NTFS_CACHE_H */