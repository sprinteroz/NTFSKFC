/*
 *
 * Author: Darryl Bennett, owner of MagDriveX
 * Created: January 6, 2026
 * NTFSPLUS Kernel Module - Attribute Handling
 * Converted from ntfs-3g attrib.c to kernel space
 * GPL Compliant - Kernel Space Adaptation
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/string.h>
#include "kernel_types.h"
#include "kernel_logging.h"

/* Attribute flags - kernel compatible */
#define ATTR_FLAG_COMPRESSED     0x0001
#define ATTR_FLAG_ENCRYPTED      0x4000
#define ATTR_FLAG_SPARSE         0x8000

/* Kernel space attribute structure - adapted from ntfs_attr */
struct ntfsplus_attr {
    struct ntfsplus_inode *ni;        /* Parent inode */
    ATTR_TYPES type;                  /* Attribute type */
    ntfschar *name;                   /* Attribute name */
    u32 name_len;                     /* Name length */

    /* Attribute data */
    u16 data_flags;                   /* Data flags */
    s64 data_size;                    /* Data size */
    s64 allocated_size;               /* Allocated size */
    s64 initialized_size;             /* Initialized size */
    s64 compressed_size;              /* Compressed size */

    /* Runlist for non-resident attributes */
    runlist_element *rl;              /* Runlist */

    /* Compression info */
    u8 compression_unit;              /* Compression unit */
    u32 compression_block_size;       /* Compression block size */
    u32 compression_block_clusters;   /* Compression block clusters */

    /* State flags */
    unsigned long state;              /* State flags */
};

/* State flags */
#define NTFSPLUS_ATTR_NON_RESIDENT       0x0001
#define NTFSPLUS_ATTR_COMPRESSED         0x0002
#define NTFSPLUS_ATTR_ENCRYPTED          0x0004
#define NTFSPLUS_ATTR_SPARSE             0x0008
#define NTFSPLUS_ATTR_RUNLIST_DIRTY      0x0010
#define NTFSPLUS_ATTR_RUNLIST_MAPPED     0x0020
#define NTFSPLUS_ATTR_COMPR_CLOSING      0x0040
#define NTFSPLUS_ATTR_DATA_APPENDING     0x0080
#define NTFSPLUS_ATTR_BEING_NON_RESIDENT 0x0100

/* Forward declarations */
static int ntfsplus_attr_map_runlist(struct ntfsplus_attr *na, VCN vcn);

/*
 * ntfsplus_attr_alloc - Allocate kernel NTFS attribute structure
 */
struct ntfsplus_attr *ntfsplus_attr_alloc(void)
{
    struct ntfsplus_attr *na;

    na = kzalloc(sizeof(*na), GFP_KERNEL);
    if (!na)
        return ERR_PTR(-ENOMEM);

    return na;
}

/*
 * ntfsplus_attr_free - Free kernel NTFS attribute structure
 */
void ntfsplus_attr_free(struct ntfsplus_attr *na)
{
    if (!na)
        return;

    if (na->rl)
        kfree(na->rl);

    if (na->name && na->name != AT_UNNAMED)
        kfree(na->name);

    kfree(na);
}

/*
 * ntfsplus_attr_open - Open an NTFS attribute for access
 * Kernel equivalent of ntfs_attr_open
 */
struct ntfsplus_attr *ntfsplus_attr_open(struct ntfsplus_inode *ni,
                                       ATTR_TYPES type,
                                       ntfschar *name, u32 name_len)
{
    struct ntfsplus_attr *na;
    int ret;

    ntfsplus_log_enter("Opening attr 0x%x for inode %llu",
                      le32_to_cpu(type), (unsigned long long)ni->mft_no);

    if (!ni || !ni->vol) {
        ntfsplus_log_error("Invalid inode or volume");
        return ERR_PTR(-EINVAL);
    }

    na = ntfsplus_attr_alloc();
    if (IS_ERR(na))
        return na;

    /* Initialize basic fields */
    na->ni = ni;
    na->type = type;
    na->name = name;
    na->name_len = name_len;

    /* For now, assume unnamed data attribute */
    if (!name || name == AT_UNNAMED) {
        na->name = AT_UNNAMED;
        na->name_len = 0;
    }

    /* Set default sizes for testing */
    na->data_size = 0;
    na->allocated_size = 0;
    na->initialized_size = 0;

    /* Set basic state */
    na->state = NTFSPLUS_ATTR_RUNLIST_MAPPED;

    ntfsplus_log_leave("Attribute opened successfully");
    return na;
}

/*
 * ntfsplus_attr_close - Close an NTFS attribute
 * Kernel equivalent of ntfs_attr_close
 */
void ntfsplus_attr_close(struct ntfsplus_attr *na)
{
    if (!na)
        return;

    ntfsplus_log_enter("Closing attribute");

    /* Free runlist if present */
    if (na->rl && (na->state & NTFSPLUS_ATTR_RUNLIST_DIRTY)) {
        /* TODO: Update mapping pairs on disk */
        ntfsplus_log_debug("Runlist was dirty, should update mapping pairs");
    }

    ntfsplus_attr_free(na);
    ntfsplus_log_leave("Attribute closed");
}

/*
 * ntfsplus_attr_pread - Read from an NTFS attribute
 * Kernel equivalent of ntfs_attr_pread
 */
s64 ntfsplus_attr_pread(struct ntfsplus_attr *na, s64 pos, s64 count, void *buf)
{
    s64 bytes_read = 0;

    ntfsplus_log_enter("Reading %lld bytes at pos %lld", (long long)count, (long long)pos);

    if (!na || !buf || pos < 0 || count < 0) {
        ntfsplus_log_error("Invalid parameters");
        return -EINVAL;
    }

    /* For resident attributes (simple case) */
    if (!(na->state & NTFSPLUS_ATTR_NON_RESIDENT)) {
        /* TODO: Read from MFT record */
        ntfsplus_log_debug("Reading from resident attribute");
        if (pos + count > na->data_size)
            count = na->data_size - pos;
        if (count > 0) {
            /* Simulate reading zeros for now */
            memset(buf, 0, count);
            bytes_read = count;
        }
    } else {
        /* Non-resident attribute */
        ntfsplus_log_debug("Reading from non-resident attribute");

        /* TODO: Implement proper runlist-based reading */
        /* For now, return zeros */
        if (pos + count > na->data_size)
            count = na->data_size - pos;
        if (count > 0) {
            memset(buf, 0, count);
            bytes_read = count;
        }
    }

    ntfsplus_log_leave("Read %lld bytes", (long long)bytes_read);
    return bytes_read;
}

/*
 * ntfsplus_attr_pwrite - Write to an NTFS attribute
 * Kernel equivalent of ntfs_attr_pwrite
 */
s64 ntfsplus_attr_pwrite(struct ntfsplus_attr *na, s64 pos, s64 count, const void *buf)
{
    s64 bytes_written = 0;

    ntfsplus_log_enter("Writing %lld bytes at pos %lld", (long long)count, (long long)pos);

    if (!na || !buf || pos < 0 || count < 0) {
        ntfsplus_log_error("Invalid parameters");
        return -EINVAL;
    }

    /* For resident attributes (simple case) */
    if (!(na->state & NTFSPLUS_ATTR_NON_RESIDENT)) {
        /* TODO: Write to MFT record */
        ntfsplus_log_debug("Writing to resident attribute");
        bytes_written = count; /* Simulate success */
    } else {
        /* Non-resident attribute */
        ntfsplus_log_debug("Writing to non-resident attribute");

        /* TODO: Implement proper runlist-based writing */
        bytes_written = count; /* Simulate success */
    }

    /* Update sizes */
    if (pos + count > na->data_size) {
        na->data_size = pos + count;
        na->initialized_size = na->data_size;
    }

    ntfsplus_log_leave("Wrote %lld bytes", (long long)bytes_written);
    return bytes_written;
}

/*
 * ntfsplus_attr_map_runlist - Map runlist for an attribute
 * Kernel equivalent of ntfs_attr_map_runlist
 */
static int ntfsplus_attr_map_runlist(struct ntfsplus_attr *na, VCN vcn)
{
    ntfsplus_log_enter("Mapping runlist for vcn %lld", (long long)vcn);

    if (!na || !(na->state & NTFSPLUS_ATTR_NON_RESIDENT)) {
        ntfsplus_log_debug("Attribute is resident or invalid");
        return 0;
    }

    /* TODO: Implement runlist mapping from attribute records */
    /* For now, assume runlist is already mapped */

    ntfsplus_log_leave("Runlist mapping completed");
    return 0;
}

/*
 * ntfsplus_attr_vcn_to_lcn - Convert VCN to LCN
 * Kernel equivalent of ntfs_attr_vcn_to_lcn
 */
LCN ntfsplus_attr_vcn_to_lcn(struct ntfsplus_attr *na, VCN vcn)
{
    LCN lcn;

    ntfsplus_log_enter("Converting VCN %lld to LCN", (long long)vcn);

    if (!na || vcn < 0) {
        ntfsplus_log_error("Invalid parameters");
        return (LCN)LCN_EINVAL;
    }

    if (!(na->state & NTFSPLUS_ATTR_NON_RESIDENT)) {
        /* Resident attribute - no conversion needed */
        ntfsplus_log_debug("Resident attribute, no conversion needed");
        return (LCN)LCN_EINVAL;
    }

    /* TODO: Implement proper VCN to LCN conversion using runlist */
    lcn = (LCN)vcn; /* Placeholder - should use runlist */

    ntfsplus_log_leave("VCN %lld -> LCN %lld", (long long)vcn, (long long)lcn);
    return lcn;
}

/*
 * ntfsplus_attr_get_data_size - Get data size of attribute
 */
s64 ntfsplus_attr_get_data_size(struct ntfsplus_attr *na)
{
    return na ? na->data_size : 0;
}

/*
 * ntfsplus_attr_set_data_size - Set data size of attribute
 */
void ntfsplus_attr_set_data_size(struct ntfsplus_attr *na, s64 size)
{
    if (na) {
        na->data_size = size;
        if (na->initialized_size > size)
            na->initialized_size = size;
    }
}

/*
 * ntfsplus_attr_is_non_resident - Check if attribute is non-resident
 */
bool ntfsplus_attr_is_non_resident(struct ntfsplus_attr *na)
{
    return na && (na->state & NTFSPLUS_ATTR_NON_RESIDENT);
}

/*
 * ntfsplus_attr_is_compressed - Check if attribute is compressed
 */
bool ntfsplus_attr_is_compressed(struct ntfsplus_attr *na)
{
    return na && (na->state & NTFSPLUS_ATTR_COMPRESSED);
}

/*
 * ntfsplus_attr_is_encrypted - Check if attribute is encrypted
 */
bool ntfsplus_attr_is_encrypted(struct ntfsplus_attr *na)
{
    return na && (na->state & NTFSPLUS_ATTR_ENCRYPTED);
}

/*
 * ntfsplus_attr_is_sparse - Check if attribute is sparse
 */
bool ntfsplus_attr_is_sparse(struct ntfsplus_attr *na)
{
    return na && (na->state & NTFSPLUS_ATTR_SPARSE);
}

/*
 * ntfs_attr_pread - Compatibility wrapper for ntfsplus_attr_pread
 */
s64 ntfs_attr_pread(struct ntfsplus_attr *na, s64 pos, s64 count, void *b)
{
    return ntfsplus_attr_pread(na, pos, count, b);
}

/*
 * ntfs_attr_pwrite - Compatibility wrapper for ntfsplus_attr_pwrite
 */
s64 ntfs_attr_pwrite(struct ntfsplus_attr *na, s64 pos, s64 count, const void *b)
{
    return ntfsplus_attr_pwrite(na, pos, count, b);
}