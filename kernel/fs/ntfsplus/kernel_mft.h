/*
 * NTFSPLUS Kernel Module - MFT Header
 * Function declarations for MFT operations
 * GPL Compliant - Kernel Space Adaptation
 */

#ifndef _KERNEL_NTFS_MFT_H
#define _KERNEL_NTFS_MFT_H

#include "kernel_types.h"

/* MFT record operations */
int ntfs_mft_records_read(const struct ntfsplus_volume *vol, const MFT_REF mref,
		const s64 count, MFT_RECORD *b);
int ntfs_mft_records_write(const struct ntfsplus_volume *vol, const MFT_REF mref,
		const s64 count, MFT_RECORD *b);
int ntfs_mft_record_check(const struct ntfsplus_volume *vol, const MFT_REF mref,
			 MFT_RECORD *m);
int ntfs_file_record_read(const struct ntfsplus_volume *vol, const MFT_REF mref,
		MFT_RECORD **mrec, ATTR_RECORD **attr);
int ntfs_mft_record_layout(const struct ntfsplus_volume *vol, const MFT_REF mref,
		MFT_RECORD *mrec);
int ntfs_mft_record_format(const struct ntfsplus_volume *vol, const MFT_REF mref);

/* MFT record allocation */
struct ntfsplus_inode *ntfs_mft_record_alloc(const struct ntfsplus_volume *vol,
                                           struct ntfsplus_inode *base_ni);
int ntfs_mft_record_free(const struct ntfsplus_volume *vol, struct ntfsplus_inode *ni);

#endif /* _KERNEL_NTFS_MFT_H */