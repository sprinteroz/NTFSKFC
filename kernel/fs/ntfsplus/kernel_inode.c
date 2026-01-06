/*
 *
 * Author: Darryl Bennett, owner of MagDriveX
 * Created: January 6, 2026
 * NTFSPLUS Kernel Module - Inode Management
 * Converted from ntfs-3g inode.c to kernel space
 * GPL Compliant - Kernel Space Adaptation
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/string.h>
#include "kernel_types.h"
#include "kernel_logging.h"
#include "kernel_mft.h"
#include "kernel_attrib.h"

/* Forward declarations */
int ntfsplus_inode_sync(struct ntfsplus_inode *ni);

/* Inode state flags */
#define NTFSPLUS_INODE_DIRTY         0x0001
#define NTFSPLUS_INODE_FILE_NAME_DIRTY 0x0002
#define NTFSPLUS_INODE_ATTR_LIST_DIRTY 0x0004
#define NTFSPLUS_INODE_TIMES_SET     0x0008
#define NTFSPLUS_INODE_V3_EXTENSIONS 0x0010

/**
 * ntfsplus_inode_allocate - Allocate a new kernel NTFS inode structure
 */
struct ntfsplus_inode *ntfsplus_inode_allocate(const struct ntfsplus_volume *vol)
{
    struct ntfsplus_inode *ni;

    ntfsplus_log_enter("Allocating inode");

    ni = kzalloc(sizeof(*ni), GFP_KERNEL);
    if (!ni)
        return ERR_PTR(-ENOMEM);

    ni->vol = (struct ntfsplus_volume *)vol;

    /* Initialize basic fields */
    ni->flags = 0;
    ni->data_size = 0;
    ni->allocated_size = 0;
    ni->nr_extents = 0;
    ni->extent_nis = NULL;
    ni->base_ni = NULL;

    /* Initialize mutex */
    mutex_init(&ni->lock);

    ntfsplus_log_leave("Inode allocated successfully");
    return ni;
}

/**
 * ntfsplus_inode_free - Free a kernel NTFS inode structure
 */
void ntfsplus_inode_free(struct ntfsplus_inode *ni)
{
    if (!ni)
        return;

    ntfsplus_log_enter("Freeing inode %lld", (long long)ni->mft_no);

    /* Free extent inodes if any */
    if (ni->extent_nis) {
        int i;
        for (i = 0; i < ni->nr_extents; i++) {
            if (ni->extent_nis[i])
                ntfsplus_inode_free(ni->extent_nis[i]);
        }
        kfree(ni->extent_nis);
    }

    /* Free MFT record */
    if (ni->mrec)
        kfree(ni->mrec);

    /* Free the inode structure */
    kfree(ni);

    ntfsplus_log_leave("Inode freed");
}

/**
 * ntfsplus_inode_open - Open an inode by reading it from disk
 * @vol:    volume to read from
 * @mref:   MFT reference of the inode to open
 */
struct ntfsplus_inode *ntfsplus_inode_open(const struct ntfsplus_volume *vol,
                                         MFT_REF mref)
{
    struct ntfsplus_inode *ni = NULL;
    ATTR_RECORD *attr = NULL;
    int ret;

    ntfsplus_log_enter("Opening inode %llu", (unsigned long long)MREF(mref));

    if (!vol) {
        ntfsplus_log_error("Invalid volume parameter");
        return ERR_PTR(-EINVAL);
    }

    /* Allocate inode structure */
    ni = ntfsplus_inode_allocate(vol);
    if (IS_ERR(ni))
        return ni;

    /* Read the file record from disk */
    ret = ntfs_file_record_read(vol, mref, &ni->mrec, &attr);
    if (ret) {
        ntfsplus_log_error("Failed to read file record %llu", (unsigned long long)MREF(mref));
        ntfsplus_inode_free(ni);
        return ERR_PTR(-EIO);
    }

    /* Set basic inode information */
    ni->mft_no = MREF(mref);

    /* Check if it's a directory */
    if (ni->mrec->flags & MFT_RECORD_IS_DIRECTORY)
        ni->flags |= FILE_ATTR_DIRECTORY;

    /* Extract standard information if available */
    if (attr && attr->type == AT_STANDARD_INFORMATION) {
        STANDARD_INFORMATION *std_info;
        std_info = (STANDARD_INFORMATION *)((u8*)attr +
                   le16_to_cpu(attr->data.resident.value_offset));

        ni->flags = std_info->file_attributes;
        ni->creation_time = std_info->creation_time;
        ni->last_data_change_time = std_info->last_data_change_time;
        ni->last_mft_change_time = std_info->last_mft_change_time;
        ni->last_access_time = std_info->last_access_time;
    }

    /* Extract data size information */
    /* For now, assume we have data attribute */
    ni->data_size = 0; /* Will be updated when data attr is accessed */
    ni->allocated_size = 0;

    ntfsplus_log_leave("Successfully opened inode %llu", (unsigned long long)MREF(mref));
    return ni;
}

/**
 * ntfsplus_inode_close - Close an inode and write it back if dirty
 * @ni:    inode to close
 */
int ntfsplus_inode_close(struct ntfsplus_inode *ni)
{
    int ret = 0;

    ntfsplus_log_enter("Closing inode %lld", (long long)ni->mft_no);

    if (!ni)
        return 0;

    /* Sync the inode if it's dirty */
    if (ni->flags & NTFSPLUS_INODE_DIRTY) {
        ret = ntfsplus_inode_sync(ni);
        if (ret) {
            ntfsplus_log_error("Failed to sync inode %lld", (long long)ni->mft_no);
        }
    }

    /* Free extent inodes */
    if (ni->nr_extents > 0) {
        int i;
        for (i = 0; i < ni->nr_extents; i++) {
            if (ni->extent_nis[i])
                ntfsplus_inode_close(ni->extent_nis[i]);
        }
    }

    /* Free the inode */
    ntfsplus_inode_free(ni);

    ntfsplus_log_leave("Inode closed %s", ret ? "with errors" : "successfully");
    return ret;
}

/**
 * ntfsplus_inode_sync - Sync an inode to disk
 * @ni:    inode to sync
 */
int ntfsplus_inode_sync(struct ntfsplus_inode *ni)
{
    int ret = 0;

    ntfsplus_log_enter("Syncing inode %lld", (long long)ni->mft_no);

    if (!ni || !ni->mrec) {
        ntfsplus_log_error("Invalid inode for sync");
        return -EINVAL;
    }

    /* Clear dirty flag */
    ni->flags &= ~NTFSPLUS_INODE_DIRTY;

    /* Write the MFT record back */
    ret = ntfs_mft_records_write(ni->vol, ni->mft_no, 1, ni->mrec);
    if (ret) {
        ntfsplus_log_error("Failed to write MFT record for inode %lld", (long long)ni->mft_no);
        /* Restore dirty flag */
        ni->flags |= NTFSPLUS_INODE_DIRTY;
        return -EIO;
    }

    ntfsplus_log_leave("Inode synced successfully");
    return 0;
}

/**
 * ntfsplus_inode_mark_dirty - Mark an inode as dirty
 * @ni:    inode to mark dirty
 */
void ntfsplus_inode_mark_dirty(struct ntfsplus_inode *ni)
{
    if (ni) {
        ni->flags |= NTFSPLUS_INODE_DIRTY;
        /* If this is an extent inode, mark the base inode dirty too */
        if (ni->base_ni)
            ni->base_ni->flags |= NTFSPLUS_INODE_DIRTY;
    }
}

/**
 * ntfsplus_inode_is_dirty - Check if inode is dirty
 * @ni:    inode to check
 */
bool ntfsplus_inode_is_dirty(const struct ntfsplus_inode *ni)
{
    return ni && (ni->flags & NTFSPLUS_INODE_DIRTY);
}

/**
 * ntfsplus_inode_base - Get the base inode for an extent inode
 * @ni:    inode (extent or base)
 */
struct ntfsplus_inode *ntfsplus_inode_base(struct ntfsplus_inode *ni)
{
    return (ni && ni->nr_extents == -1) ? ni->base_ni : ni;
}

/**
 * ntfsplus_inode_attach_extent - Attach an extent inode to a base inode
 * @base_ni:    base inode
 * @extent_ni:  extent inode to attach
 */
int ntfsplus_inode_attach_extent(struct ntfsplus_inode *base_ni,
                                struct ntfsplus_inode *extent_ni)
{
    struct ntfsplus_inode **new_extent_nis;
    int new_nr_extents;

    ntfsplus_log_enter("Attaching extent inode %lld to base %lld",
                      (long long)extent_ni->mft_no, (long long)base_ni->mft_no);

    if (!base_ni || !extent_ni) {
        ntfsplus_log_error("Invalid parameters for extent attachment");
        return -EINVAL;
    }

    /* Allocate space for the new extent */
    new_nr_extents = base_ni->nr_extents + 1;
    new_extent_nis = krealloc(base_ni->extent_nis,
                             new_nr_extents * sizeof(struct ntfsplus_inode *),
                             GFP_KERNEL);
    if (!new_extent_nis) {
        ntfsplus_log_error("Failed to allocate extent array");
        return -ENOMEM;
    }

    /* Add the extent inode */
    new_extent_nis[base_ni->nr_extents] = extent_ni;
    base_ni->extent_nis = new_extent_nis;
    base_ni->nr_extents = new_nr_extents;

    /* Set up extent inode */
    extent_ni->base_ni = base_ni;
    extent_ni->nr_extents = -1; /* Mark as extent */

    ntfsplus_log_leave("Extent inode attached successfully");
    return 0;
}

/**
 * ntfsplus_inode_detach_extent - Detach an extent inode from its base
 * @extent_ni:  extent inode to detach
 */
int ntfsplus_inode_detach_extent(struct ntfsplus_inode *extent_ni)
{
    struct ntfsplus_inode *base_ni;
    int i, found = -1;

    ntfsplus_log_enter("Detaching extent inode %lld", (long long)extent_ni->mft_no);

    if (!extent_ni || !extent_ni->base_ni) {
        ntfsplus_log_error("Invalid extent inode for detachment");
        return -EINVAL;
    }

    base_ni = extent_ni->base_ni;

    /* Find the extent in the base inode's array */
    for (i = 0; i < base_ni->nr_extents; i++) {
        if (base_ni->extent_nis[i] == extent_ni) {
            found = i;
            break;
        }
    }

    if (found == -1) {
        ntfsplus_log_error("Extent inode not found in base inode's extent array");
        return -ENOENT;
    }

    /* Remove the extent from the array */
    for (i = found; i < base_ni->nr_extents - 1; i++) {
        base_ni->extent_nis[i] = base_ni->extent_nis[i + 1];
    }

    base_ni->nr_extents--;

    /* Shrink the array if needed */
    if (base_ni->nr_extents > 0) {
        struct ntfsplus_inode **new_extent_nis;
        new_extent_nis = krealloc(base_ni->extent_nis,
                                 base_ni->nr_extents * sizeof(struct ntfsplus_inode *),
                                 GFP_KERNEL);
        if (new_extent_nis)
            base_ni->extent_nis = new_extent_nis;
    } else {
        kfree(base_ni->extent_nis);
        base_ni->extent_nis = NULL;
    }

    /* Clear extent inode's base reference */
    extent_ni->base_ni = NULL;
    extent_ni->nr_extents = 0; /* Mark as regular inode again */

    ntfsplus_log_leave("Extent inode detached successfully");
    return 0;
}

/**
 * ntfsplus_inode_update_times - Update inode timestamps
 * @ni:     inode to update
 * @mask:   mask of which times to update
 */
void ntfsplus_inode_update_times(struct ntfsplus_inode *ni, int mask)
{
    u64 now;

    if (!ni)
        return;

    /* Get current time (simplified - should use proper NTFS time) */
    now = jiffies; /* Placeholder */

    if (mask & NTFS_UPDATE_ATIME)
        ni->last_access_time = cpu_to_sle64(now);

    if (mask & NTFS_UPDATE_MTIME)
        ni->last_data_change_time = cpu_to_sle64(now);

    if (mask & NTFS_UPDATE_CTIME)
        ni->last_mft_change_time = cpu_to_sle64(now);

    /* Mark as dirty */
    ntfsplus_inode_mark_dirty(ni);
}

/**
 * ntfsplus_inode_get_size - Get the data size of an inode
 * @ni:    inode to query
 */
s64 ntfsplus_inode_get_size(const struct ntfsplus_inode *ni)
{
    return ni ? ni->data_size : 0;
}

/**
 * ntfsplus_inode_set_size - Set the data size of an inode
 * @ni:     inode to modify
 * @size:   new data size
 */
void ntfsplus_inode_set_size(struct ntfsplus_inode *ni, s64 size)
{
    if (ni) {
        ni->data_size = size;
        ntfsplus_inode_mark_dirty(ni);
    }
}

/**
 * ntfsplus_inode_is_directory - Check if inode represents a directory
 * @ni:    inode to check
 */
bool ntfsplus_inode_is_directory(const struct ntfsplus_inode *ni)
{
    return ni && (ni->flags & FILE_ATTR_DIRECTORY);
}

/**
 * ntfsplus_inode_is_system - Check if inode represents a system file
 * @ni:    inode to check
 */
bool ntfsplus_inode_is_system(const struct ntfsplus_inode *ni)
{
    return ni && (ni->mft_no < 16);
}

/**
 * ntfsplus_inode_get_flags - Get inode flags
 * @ni:    inode to query
 */
u32 ntfsplus_inode_get_flags(const struct ntfsplus_inode *ni)
{
    return ni ? ni->flags : 0;
}

/**
 * ntfsplus_inode_set_flags - Set inode flags
 * @ni:     inode to modify
 * @flags:  new flags
 */
void ntfsplus_inode_set_flags(struct ntfsplus_inode *ni, u32 flags)
{
    if (ni) {
        ni->flags = flags;
        ntfsplus_inode_mark_dirty(ni);
    }
}