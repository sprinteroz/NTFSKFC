/*
 * NTFSPLUS Kernel Module - Core Types and Structures
 * Derived from ntfs-3g: types.h, layout.h, compat.h
 * GPL Compliant - Kernel Space Adaptation
 */

#ifndef _KERNEL_NTFS_TYPES_H
#define _KERNEL_NTFS_TYPES_H

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <asm/byteorder.h>

/* Basic NTFS types - kernel safe */
typedef __le16 le16;
typedef __le32 le32;
typedef __le64 le64;
typedef __be16 be16;
typedef __be32 be32;
typedef __be64 be64;

/* Unicode character type */
typedef __le16 ntfschar;

/* Special constants */
#define const_le_to_cpu(x) __le32_to_cpu(x)
#define const_cpu_to_le(x) __cpu_to_le32(x)
#define const_cpu_to_le16(x) __cpu_to_le16(x)

/* Endian conversion macros - kernel safe */
#define le16_to_cpu(x) __le16_to_cpu(x)
#define le32_to_cpu(x) __le32_to_cpu(x)
#define le64_to_cpu(x) __le64_to_cpu(x)
#define cpu_to_le16(x) __cpu_to_le16(x)
#define cpu_to_le32(x) __cpu_to_le32(x)
#define cpu_to_le64(x) __cpu_to_le64(x)

/* Signed endian conversion */
#define sle16_to_cpu(x) (__s16)__le16_to_cpu(x)
#define sle32_to_cpu(x) (__s32)__le32_to_cpu(x)
#define sle64_to_cpu(x) (__s64)__le64_to_cpu(x)
#define cpu_to_sle16(x) (__le16)(__s16)(x)
#define cpu_to_sle32(x) (__le32)(__s32)(x)
#define cpu_to_sle64(x) (__le64)(__s64)(x)

/* Signed little-endian types */
typedef __s16 sle16;
typedef __s32 sle32;
typedef __s64 sle64;

/* NTFS attribute types */
typedef le32 ATTR_TYPES;

#define AT_UNUSED                        __cpu_to_le32(0)
#define AT_STANDARD_INFORMATION          __cpu_to_le32(0x10)
#define AT_ATTRIBUTE_LIST                __cpu_to_le32(0x20)
#define AT_FILE_NAME                     __cpu_to_le32(0x30)
#define AT_OBJECT_ID                     __cpu_to_le32(0x40)
#define AT_SECURITY_DESCRIPTOR           __cpu_to_le32(0x50)
#define AT_VOLUME_NAME                   __cpu_to_le32(0x60)
#define AT_VOLUME_INFORMATION            __cpu_to_le32(0x70)
#define AT_DATA                          __cpu_to_le32(0x80)
#define AT_INDEX_ROOT                    __cpu_to_le32(0x90)
#define AT_INDEX_ALLOCATION              __cpu_to_le32(0xA0)
#define AT_BITMAP                        __cpu_to_le32(0xB0)
#define AT_REPARSE_POINT                 __cpu_to_le32(0xC0)
#define AT_EA_INFORMATION                __cpu_to_le32(0xD0)
#define AT_EA                            __cpu_to_le32(0xE0)
#define AT_PROPERTY_SET                  __cpu_to_le32(0xF0)
#define AT_LOGGED_UTILITY_STREAM         __cpu_to_le32(0x100)
#define AT_FIRST_USER_DEFINED_ATTRIBUTE  __cpu_to_le32(0x1000)
#define AT_END                           __cpu_to_le32(0xFFFFFFFF)

/* NTFS attribute flags */
typedef le16 ATTR_FLAGS;
#define ATTR_IS_COMPRESSED     __cpu_to_le16(0x0001)
#define ATTR_COMPRESSION_MASK  __cpu_to_le16(0x00FF)
#define ATTR_IS_ENCRYPTED      __cpu_to_le16(0x4000)
#define ATTR_IS_SPARSE         __cpu_to_le16(0x8000)

/* MFT reference structure */
typedef le64 MFT_REF;
typedef le64 MREF;

/* File permissions */
typedef le32 FILE_ATTR_FLAGS;
#define FILE_ATTR_READONLY              __cpu_to_le32(0x00000001)
#define FILE_ATTR_HIDDEN                __cpu_to_le32(0x00000002)
#define FILE_ATTR_SYSTEM                __cpu_to_le32(0x00000004)
#define FILE_ATTR_DIRECTORY             __cpu_to_le32(0x00000010)
#define FILE_ATTR_ARCHIVE               __cpu_to_le32(0x00000020)
#define FILE_ATTR_DEVICE                __cpu_to_le32(0x00000040)
#define FILE_ATTR_NORMAL                __cpu_to_le32(0x00000080)
#define FILE_ATTR_TEMPORARY             __cpu_to_le32(0x00000100)
#define FILE_ATTR_SPARSE_FILE           __cpu_to_le32(0x00000200)
#define FILE_ATTR_REPARSE_POINT         __cpu_to_le32(0x00000400)
#define FILE_ATTR_COMPRESSED            __cpu_to_le32(0x00000800)
#define FILE_ATTR_OFFLINE               __cpu_to_le32(0x00001000)
#define FILE_ATTR_NOT_CONTENT_INDEXED   __cpu_to_le32(0x00002000)
#define FILE_ATTR_ENCRYPTED             __cpu_to_le32(0x00004000)
#define FILE_ATTR_NOT_CONTENT_INDEXED2  __cpu_to_le32(0x00008000)
#define FILE_ATTR_NOT_CONTENT_INDEXED3  __cpu_to_le32(0x00010000)
#define FILE_ATTR_NOT_CONTENT_INDEXED4  __cpu_to_le32(0x00020000)
#define FILE_ATTR_INTEGRITY_STREAM      __cpu_to_le32(0x00080000)
#define FILE_ATTR_VIRTUAL               __cpu_to_le32(0x00100000)
#define FILE_ATTR_NO_SCRUB_DATA         __cpu_to_le32(0x00200000)

/* Core NTFS structures - kernel safe */

/* NTFS boot sector */
typedef struct {
u8 jump[3];
u8 oem_id[8];
le16 bytes_per_sector;
u8 sectors_per_cluster;
le16 reserved_sectors;
u8 fats;
le16 root_entries;
le16 sectors;
u8 media_type;
le16 sectors_per_fat;
le16 sectors_per_track;
le16 heads;
le32 hidden_sectors;
le32 total_sectors;
le64 mft_lcn;
le64 mftmirr_lcn;
s8 clusters_per_mft_record;
u8 reserved1[3];
s8 clusters_per_index_record;
u8 reserved2[3];
le64 volume_serial_number;
le32 checksum;
u8 bootstrap[426];
le16 end_of_sector_marker;
} __attribute__((packed)) NTFS_BOOT_SECTOR;

/* MFT record header */
typedef struct {
le32 magic;
le16 usa_ofs;
le16 usa_count;
le64 lsn;
le16 sequence_number;
le16 link_count;
le16 attrs_offset;
le16 flags;
le32 bytes_in_use;
le32 bytes_allocated;
le64 base_mft_record;
le16 next_attr_instance;
le16 reserved;
le32 mft_record_number;
} __attribute__((packed)) MFT_RECORD;

/* Attribute record header */
typedef struct {
    le32 type;
    le32 length;
    u8 non_resident;
    u8 name_length;
    le16 name_offset;
    le16 flags;
    le16 instance;
    union {
        struct {
            le32 value_length;
            le16 value_offset;
            le16 resident_flags;
        } __attribute__((packed)) resident;
        struct {
            le64 lowest_vcn;
            le64 highest_vcn;
            le16 mapping_pairs_offset;
            le16 compression_unit;
            u8 reserved1[4];
            le64 allocated_size;
            le64 data_size;
            le64 initialized_size;
            le64 compressed_size;
        } __attribute__((packed)) non_resident;
    } __attribute__((packed)) data;
} __attribute__((packed)) ATTR_RECORD;

/* File name attribute */
typedef struct {
le64 parent_directory;
le64 creation_time;
le64 last_data_change_time;
le64 last_mft_change_time;
le64 last_access_time;
le64 allocated_size;
le64 data_size;
le32 file_attributes;
le32 alignment_or_reserved;
u8 name_length;
u8 name_type;
le16 name[0];
} __attribute__((packed)) FILE_NAME_ATTR;

/* Standard information attribute */
typedef struct {
le64 creation_time;
le64 last_data_change_time;
le64 last_mft_change_time;
le64 last_access_time;
le32 file_attributes;
le32 maximum_versions;
le32 version_number;
le32 class_id;
le32 owner_id;
le32 security_id;
le64 quota_charged;
le64 usn;
} __attribute__((packed)) STANDARD_INFORMATION;

/* Index root structure */
typedef struct {
le32 type;
le32 collation_rule;
le32 index_block_size;
s8 clusters_per_index_block;
u8 reserved[3];
} __attribute__((packed)) INDEX_ROOT;

/* Index header */
typedef struct {
le32 entries_offset;
le32 index_length;
le32 allocated_size;
le32 flags;
} __attribute__((packed)) INDEX_HEADER;

/* Index entry */
typedef struct {
le64 mft_reference;
le16 length;
le16 key_length;
le16 flags;
le16 reserved;
} __attribute__((packed)) INDEX_ENTRY;

/* Virtual Cluster Number and Logical Cluster Number types */
typedef s64 VCN;
typedef s64 LCN;

/* Runlist element */
typedef struct {
    VCN vcn;
    LCN lcn;
    s64 length;
} runlist_element;

/* Volume information */
typedef struct {
le64 reserved;
u8 major_ver;
u8 minor_ver;
le16 flags;
} __attribute__((packed)) VOLUME_INFORMATION;

/* Constants */
#define NTFS_BLOCK_SIZE     512
#define NTFS_SECTOR_SIZE    512
#define NTFS_BUFFER_SIZE    4096

/* Special attribute names */
#define AT_UNNAMED          ((ntfschar *)NULL)
#define NTFS_INDEX_I30      __cpu_to_le16('I') | (__cpu_to_le16('3') << 16) | (__cpu_to_le16('0') << 32)

/* LCN constants for runlist */
#define LCN_HOLE            ((LCN)-1)
#define LCN_RL_NOT_MAPPED   ((LCN)-2)
#define LCN_ENOENT          ((LCN)-3)
#define LCN_EINVAL          ((LCN)-4)
#define LCN_EIO             ((LCN)-5)

#define MFT_RECORD_MAGIC    __cpu_to_le32(0x454C4946)  /* "FILE" */
#define INDEX_RECORD_MAGIC  __cpu_to_le32(0x58444E49)  /* "INDX" */

#define MFT_RECORD_IN_USE       __cpu_to_le16(0x0001)
#define MFT_RECORD_IS_DIRECTORY __cpu_to_le16(0x0002)

/* Special MFT record numbers */
#define FILE_MFT      0
#define FILE_MFTMirr  1
#define FILE_LogFile  2
#define FILE_Volume   3
#define FILE_AttrDef  4
#define FILE_root     5
#define FILE_Bitmap   6
#define FILE_Boot     7
#define FILE_BadClus  8
#define FILE_Secure   9
#define FILE_UpCase   10
#define FILE_Extend   11

/* NTFS file signature */
#define NTFS_FILE_SIGNATURE  __cpu_to_le32(0x454C4946)  /* "FILE" */

/* MFT reference macros */
#define MREF(x)         ((u64)(x) & 0x0000FFFFFFFFFFFFULL)
#define MSEQNO(x)       ((u16)((u64)(x) >> 48) & 0xFFFF)

/* MFT record flags */
#define MFT_RECORD_IN_USE     __cpu_to_le16(0x0001)
#define MFT_RECORD_IS_DIRECTORY __cpu_to_le16(0x0002)

/* Magic numbers */
#define magic_FILE      __cpu_to_le32(0x454C4946)  /* "FILE" */
#define magic_BAAD      __cpu_to_le32(0x44414142)  /* "BAAD" */

/* MFT reference creation */
#define MK_LE_MREF(m, s)  ((MFT_REF)((u64)(s) << 48 | (u64)(m)))

/* Forward declarations for kernel structures */
struct ntfsplus_inode;

/* NTFSPLUS volume structure - kernel space */
struct ntfsplus_volume {
    struct super_block *sb;        /* VFS superblock */
    struct block_device *bdev;     /* Block device */

    /* NTFS boot sector info */
    u32 cluster_size;              /* Cluster size in bytes */
    u64 nr_clusters;               /* Number of clusters */
    u32 mft_record_size;           /* MFT record size */
    u32 mft_record_size_bits;      /* MFT record size in bits */
    u64 mft_lcn;                   /* LCN of MFT */
    u64 mftmirr_lcn;               /* LCN of MFT mirror */

    /* Volume state */
    unsigned long flags;           /* Volume flags */
    u8 major_ver;                  /* Major version */
    u8 minor_ver;                  /* Minor version */

    /* MFT management */
    struct ntfsplus_attr *mft_na;  /* MFT data attribute */
    struct ntfsplus_attr *mftbmp_na; /* MFT bitmap attribute */
    u64 mft_data_pos;              /* Next MFT record position */

    /* Upcase table */
    ntfschar *upcase;              /* Upcase table */
    u32 upcase_len;                /* Upcase table length */

    /* Memory management */
    struct kmem_cache *mft_cache;  /* MFT record cache */

    /* Synchronization */
    struct mutex volume_mutex;     /* Volume mutex */
};

/* Time update flags */
#define NTFS_UPDATE_ATIME    0x01
#define NTFS_UPDATE_MTIME    0x02
#define NTFS_UPDATE_CTIME    0x04

/* Basic kernel NTFS inode structure - will be expanded */
struct ntfsplus_inode {
    u64 mft_no;                    /* MFT record number */
    struct ntfsplus_volume *vol;   /* Parent volume */
    MFT_RECORD *mrec;             /* MFT record */
    unsigned long flags;          /* Inode flags */
    s64 data_size;                /* Data size */
    s64 allocated_size;           /* Allocated size */
    struct mutex lock;            /* Inode lock */

    /* Timestamps */
    sle64 creation_time;          /* File creation time */
    sle64 last_data_change_time;  /* Last data modification time */
    sle64 last_mft_change_time;   /* Last MFT modification time */
    sle64 last_access_time;       /* Last access time */

    /* Extent information */
    int nr_extents;               /* Number of extents */
    struct ntfsplus_inode **extent_nis; /* Extent inodes */
    struct ntfsplus_inode *base_ni; /* Base inode */
};

#endif /* _KERNEL_NTFS_TYPES_H */