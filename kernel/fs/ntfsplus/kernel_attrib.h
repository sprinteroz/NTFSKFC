/*
 * NTFSPLUS Kernel Module - Attribute Header
 * Function declarations for attribute operations
 * GPL Compliant - Kernel Space Adaptation
 */

#ifndef _KERNEL_NTFS_ATTRIB_H
#define _KERNEL_NTFS_ATTRIB_H

#include "kernel_types.h"

/* NTFSPLUS attribute structure - kernel space */
struct ntfsplus_attr {
    ATTR_TYPES type;              /* Attribute type */
    ntfschar *name;               /* Attribute name */
    u32 name_len;                 /* Attribute name length */

    struct ntfsplus_inode *ni;    /* Parent inode */
    struct ntfsplus_volume *vol;  /* Parent volume */

    /* Attribute data */
    s64 data_size;                /* Data size */
    s64 allocated_size;           /* Allocated size */
    s64 initialized_size;         /* Initialized size */

    /* Runlist for non-resident attributes */
    runlist_element *rl;          /* Runlist */

    /* Resident data */
    void *data;                   /* Resident data buffer */

    /* Attribute flags */
    ATTR_FLAGS flags;             /* Attribute flags */

    /* Reference to attribute record */
    ATTR_RECORD *attr;            /* Attribute record in MFT */

    unsigned long state;          /* Attribute state */
};

/* Attribute operations */
s64 ntfs_attr_pread(struct ntfsplus_attr *na, s64 pos, s64 count, void *b);
s64 ntfs_attr_pwrite(struct ntfsplus_attr *na, s64 pos, s64 count, const void *b);
s64 ntfs_attr_mst_pread(struct ntfsplus_attr *na, s64 pos, s64 count, u32 bsize, void *b);
s64 ntfs_attr_mst_pwrite(struct ntfsplus_attr *na, s64 pos, s64 count, u32 bsize, const void *b);

/* Alternative names for compatibility */
#define ntfsplus_attr_pread ntfs_attr_pread
#define ntfsplus_attr_pwrite ntfs_attr_pwrite
#define ntfsplus_attr_mst_pread ntfs_attr_mst_pread
#define ntfsplus_attr_mst_pwrite ntfs_attr_mst_pwrite

/* Utility functions */
static inline bool ntfs_is_file_record(le32 magic)
{
    return magic == magic_FILE;
}

#endif /* _KERNEL_NTFS_ATTRIB_H */