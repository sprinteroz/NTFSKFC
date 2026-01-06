/*
 *
 * Author: Darryl Bennett, owner of MagDriveX
 * Created: January 6, 2026
 * NTFSPLUS Kernel Module - Master File Table Management
 * Converted from ntfs-3g mft.c to kernel space
 * GPL Compliant - Kernel Space Adaptation
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/string.h>
#include "kernel_types.h"
#include "kernel_logging.h"
#include "kernel_attrib.h"

/* MFT record read/write functions */

/**
 * ntfs_mft_records_read - read records from the mft from disk
 * @vol:	volume to read from
 * @mref:	starting mft record number to read
 * @count:	number of mft records to read
 * @b:		output data buffer
 */
int ntfs_mft_records_read(const struct ntfsplus_volume *vol, const MFT_REF mref,
		const s64 count, MFT_RECORD *b)
{
	s64 br;
	VCN m;

	ntfsplus_log_enter("Reading MFT records starting at %llu, count %lld",
	                  (unsigned long long)MREF(mref), (long long)count);

	if (!vol || !vol->mft_na || !b || count < 0) {
		ntfsplus_log_error("Invalid parameters for mft_records_read");
		return -EINVAL;
	}

	m = MREF(mref);

	/* Refuse to read non-allocated mft records. */
	if (m + count > vol->mft_na->allocated_size >> vol->mft_record_size_bits) {
		ntfsplus_log_error("Trying to read non-allocated mft records");
		return -ENOSPC;
	}

	/* Read the MFT records */
	br = ntfs_attr_pread(vol->mft_na, m << vol->mft_record_size_bits,
	                    count * vol->mft_record_size, b);

	if (br != count * vol->mft_record_size) {
		if (br < 0) {
			ntfsplus_log_error("Failed to read MFT records");
			return -EIO;
		} else {
			ntfsplus_log_error("Partial read of MFT records");
			return -EIO;
		}
	}

	ntfsplus_log_leave("Successfully read %lld MFT records", (long long)count);
	return 0;
}

/**
 * ntfs_mft_records_write - write mft records to disk
 * @vol:	volume to write to
 * @mref:	starting mft record number to write
 * @count:	number of mft records to write
 * @b:		data buffer containing the mft records to write
 */
int ntfs_mft_records_write(const struct ntfsplus_volume *vol, const MFT_REF mref,
		const s64 count, MFT_RECORD *b)
{
	s64 bw;
	VCN m;

	ntfsplus_log_enter("Writing MFT records starting at %llu, count %lld",
	                  (unsigned long long)MREF(mref), (long long)count);

	if (!vol || !vol->mft_na || !b || count < 0) {
		ntfsplus_log_error("Invalid parameters for mft_records_write");
		return -EINVAL;
	}

	m = MREF(mref);

	/* Refuse to write non-allocated mft records. */
	if (m + count > vol->mft_na->allocated_size >> vol->mft_record_size_bits) {
		ntfsplus_log_error("Trying to write non-allocated mft records");
		return -ENOSPC;
	}

	/* Write the MFT records */
	bw = ntfs_attr_pwrite(vol->mft_na, m << vol->mft_record_size_bits,
	                     count * vol->mft_record_size, b);

	if (bw != count * vol->mft_record_size) {
		if (bw < 0) {
			ntfsplus_log_error("Failed to write MFT records");
			return -EIO;
		} else {
			ntfsplus_log_error("Partial write of MFT records");
			return -EIO;
		}
	}

	ntfsplus_log_leave("Successfully wrote %lld MFT records", (long long)count);
	return 0;
}

/**
 * ntfs_mft_record_check - check the consistency of an MFT record
 * @vol:	volume to which the mft record belongs
 * @mref:	mft reference of the record
 * @m:		mft record to check
 */
int ntfs_mft_record_check(const struct ntfsplus_volume *vol, const MFT_REF mref,
			 MFT_RECORD *m)
{
	u32 biu;
	u16 attrs_offset;

	ntfsplus_log_enter("Checking MFT record %llu", (unsigned long long)MREF(mref));

	/* Check magic number */
	if (!ntfs_is_file_record(m->magic)) {
		ntfsplus_log_error("MFT record %llu has invalid magic", (unsigned long long)MREF(mref));
		return -EIO;
	}

	/* Check allocated size */
	if (le32_to_cpu(m->bytes_allocated) != vol->mft_record_size) {
		ntfsplus_log_error("MFT record %llu has wrong allocated size", (unsigned long long)MREF(mref));
		return -EIO;
	}

	/* Check bytes_in_use is aligned */
	biu = le32_to_cpu(m->bytes_in_use);
	if (biu & 7) {
		ntfsplus_log_error("MFT record %llu bytes_in_use not aligned", (unsigned long long)MREF(mref));
		return -EIO;
	}

	/* Check used size overflow */
	if (biu > vol->mft_record_size) {
		ntfsplus_log_error("MFT record %llu bytes_in_use overflow", (unsigned long long)MREF(mref));
		return -EIO;
	}

	/* Check attributes offset */
	attrs_offset = le16_to_cpu(m->attrs_offset);
	if (attrs_offset < sizeof(MFT_RECORD) - sizeof(ntfschar) * 0 ||
	    attrs_offset > vol->mft_record_size) {
		ntfsplus_log_error("MFT record %llu attributes offset invalid", (unsigned long long)MREF(mref));
		return -EIO;
	}

	ntfsplus_log_leave("MFT record %llu check passed", (unsigned long long)MREF(mref));
	return 0;
}

/**
 * ntfs_file_record_read - read a FILE record from the mft from disk
 * @vol:	volume to read from
 * @mref:	mft reference specifying mft record to read
 * @mrec:	address of pointer in which to return the mft record
 * @attr:	address of pointer in which to return the first attribute
 */
int ntfs_file_record_read(const struct ntfsplus_volume *vol, const MFT_REF mref,
		MFT_RECORD **mrec, ATTR_RECORD **attr)
{
	MFT_RECORD *m;

	ntfsplus_log_enter("Reading file record %llu", (unsigned long long)MREF(mref));

	if (!vol || !mrec) {
		ntfsplus_log_error("Invalid parameters for file_record_read");
		return -EINVAL;
	}

	/* Allocate buffer if needed */
	m = *mrec;
	if (!m) {
		m = kzalloc(vol->mft_record_size, GFP_KERNEL);
		if (!m)
			return -ENOMEM;
	}

	/* Read the MFT record */
	if (ntfs_mft_records_read(vol, mref, 1, m))
		goto err_out;

	/* Check sequence number if provided */
	if (MSEQNO(mref) && MSEQNO(mref) != le16_to_cpu(m->sequence_number)) {
		ntfsplus_log_error("MFT record %llu sequence number mismatch",
		                  (unsigned long long)MREF(mref));
		goto err_out;
	}

	/* Check record consistency */
	if (ntfs_mft_record_check(vol, mref, m))
		goto err_out;

	*mrec = m;
	if (attr)
		*attr = (ATTR_RECORD*)((char*)m + le16_to_cpu(m->attrs_offset));

	ntfsplus_log_leave("Successfully read file record %llu", (unsigned long long)MREF(mref));
	return 0;

err_out:
	if (m != *mrec)
		kfree(m);
	return -EIO;
}

/**
 * ntfs_mft_record_layout - layout an mft record into a memory buffer
 * @vol:	volume to which the mft record will belong
 * @mref:	mft reference specifying the mft record number
 * @mrec:	destination buffer of size >= @vol->mft_record_size bytes
 */
int ntfs_mft_record_layout(const struct ntfsplus_volume *vol, const MFT_REF mref,
		MFT_RECORD *mrec)
{
	ATTR_RECORD *a;

	ntfsplus_log_enter("Laying out MFT record %llu", (unsigned long long)MREF(mref));

	if (!vol || !mrec) {
		ntfsplus_log_error("Invalid parameters for mft_record_layout");
		return -EINVAL;
	}

	/* Initialize MFT record header */
	mrec->magic = magic_FILE;
	mrec->usa_ofs = cpu_to_le16((sizeof(MFT_RECORD) + 1) & ~1);
	mrec->usa_count = cpu_to_le16(vol->mft_record_size / NTFS_BLOCK_SIZE + 1);

	/* Set the update sequence number to 1. */
	*(le16*)((u8*)mrec + le16_to_cpu(mrec->usa_ofs)) = cpu_to_le16(1);

	mrec->lsn = cpu_to_le64(0);
	mrec->sequence_number = cpu_to_le16(1);
	mrec->link_count = cpu_to_le16(0);
	mrec->attrs_offset = cpu_to_le16((le16_to_cpu(mrec->usa_ofs) +
			(le16_to_cpu(mrec->usa_count) << 1) + 7) & ~7);
	mrec->flags = cpu_to_le16(0);
	mrec->bytes_in_use = cpu_to_le32((le16_to_cpu(mrec->attrs_offset) + 8 + 7) & ~7);
	mrec->bytes_allocated = cpu_to_le32(vol->mft_record_size);
	mrec->base_mft_record = cpu_to_le64((MFT_REF)0);
	mrec->next_attr_instance = cpu_to_le16(0);

	/* Add end-of-attributes marker */
	a = (ATTR_RECORD*)((u8*)mrec + le16_to_cpu(mrec->attrs_offset));
	a->type = AT_END;
	a->length = cpu_to_le32(0);

	/* Clear the unused part */
	memset((u8*)a + 8, 0, vol->mft_record_size - ((u8*)a + 8 - (u8*)mrec));

	ntfsplus_log_leave("Successfully laid out MFT record %llu", (unsigned long long)MREF(mref));
	return 0;
}

/**
 * ntfs_mft_record_format - format an mft record on an ntfs volume
 * @vol:	volume on which to format the mft record
 * @mref:	mft reference specifying mft record to format
 */
int ntfs_mft_record_format(const struct ntfsplus_volume *vol, const MFT_REF mref)
{
	MFT_RECORD *m;
	int ret;

	ntfsplus_log_enter("Formatting MFT record %llu", (unsigned long long)MREF(mref));

	m = kzalloc(vol->mft_record_size, GFP_KERNEL);
	if (!m)
		return -ENOMEM;

	if (ntfs_mft_record_layout(vol, mref, m)) {
		ret = -EINVAL;
		goto free_m;
	}

	if (ntfs_mft_records_write(vol, mref, 1, m)) {
		ret = -EIO;
		goto free_m;
	}

	ret = 0;
free_m:
	kfree(m);
	ntfsplus_log_leave("MFT record format %s", ret ? "failed" : "successful");
	return ret;
}

/**
 * ntfs_mft_bitmap_find_free_rec - find a free mft record in the mft bitmap
 * @vol:	volume on which to search for a free mft record
 * @base_ni:	open base inode if allocating an extent mft record or NULL
 */
static s64 ntfs_mft_bitmap_find_free_rec(const struct ntfsplus_volume *vol,
                                        struct ntfsplus_inode *base_ni)
{
	s64 data_pos, pass_end;
	int pass;

	ntfsplus_log_enter("Finding free MFT record");

	/* Set the end of the pass */
	pass_end = vol->mft_na->allocated_size >> vol->mft_record_size_bits;

	pass = 1;
	data_pos = vol->mft_data_pos;

	if (data_pos < 64)  /* Skip reserved records */
		data_pos = 64;

	if (data_pos >= pass_end) {
		data_pos = 64;
		pass = 2;
	}

	if (base_ni) {
		data_pos = base_ni->mft_no + 1;
		pass = 2;
	}

	/* For now, just return the next available position */
	/* TODO: Implement proper bitmap scanning */
	if (data_pos < pass_end) {
		ntfsplus_log_leave("Found free MFT record at %lld", (long long)data_pos);
		return data_pos;
	}

	ntfsplus_log_leave("No free MFT records found");
	return -ENOSPC;
}

/**
 * ntfs_mft_record_alloc - allocate an mft record on an ntfs volume
 * @vol:	volume on which to allocate the mft record
 * @base_ni:	open base inode if allocating an extent mft record or NULL
 */
struct ntfsplus_inode *ntfs_mft_record_alloc(const struct ntfsplus_volume *vol,
                                           struct ntfsplus_inode *base_ni)
{
	s64 bit;
	MFT_RECORD *m;
	struct ntfsplus_inode *ni = NULL;

	ntfsplus_log_enter("Allocating MFT record");

	bit = ntfs_mft_bitmap_find_free_rec(vol, base_ni);
	if (bit < 0) {
		ntfsplus_log_error("No free MFT record found");
		return ERR_PTR(-ENOSPC);
	}

	/* Format the MFT record */
	if (ntfs_mft_record_format(vol, bit)) {
		ntfsplus_log_error("Failed to format MFT record");
		return ERR_PTR(-EIO);
	}

	/* Read the formatted record */
	m = kzalloc(vol->mft_record_size, GFP_KERNEL);
	if (!m)
		return ERR_PTR(-ENOMEM);

	if (ntfs_mft_records_read(vol, bit, 1, m)) {
		ntfsplus_log_error("Failed to read formatted MFT record");
		kfree(m);
		return ERR_PTR(-EIO);
	}

	/* Create inode structure */
	ni = kzalloc(sizeof(*ni), GFP_KERNEL);
	if (!ni) {
		kfree(m);
		return ERR_PTR(-ENOMEM);
	}

	ni->mft_no = bit;
	ni->mrec = m;
	ni->vol = (struct ntfsplus_volume *)vol;

	/* Set flags */
	m->flags |= MFT_RECORD_IN_USE;

	if (base_ni) {
		/* Extent record */
		ni->nr_extents = -1;
		ni->base_ni = base_ni;
		m->base_mft_record = MK_LE_MREF(base_ni->mft_no,
		                               le16_to_cpu(base_ni->mrec->sequence_number));
	} else {
		/* Base record */
		ni->nr_extents = 0;
		ni->base_ni = NULL;
	}

	ntfsplus_log_leave("Successfully allocated MFT record %lld", (long long)bit);
	return ni;
}

/**
 * ntfs_mft_record_free - free an mft record on an ntfs volume
 * @vol:	volume on which to free the mft record
 * @ni:		open ntfs inode of the mft record to free
 */
int ntfs_mft_record_free(const struct ntfsplus_volume *vol, struct ntfsplus_inode *ni)
{
	ntfsplus_log_enter("Freeing MFT record %lld", (long long)ni->mft_no);

	if (!vol || !ni) {
		ntfsplus_log_error("Invalid parameters for mft_record_free");
		return -EINVAL;
	}

	/* Mark as not in use */
	ni->mrec->flags &= ~MFT_RECORD_IN_USE;

	/* Increment sequence number */
	u16 seq_no = le16_to_cpu(ni->mrec->sequence_number);
	if (seq_no == 0xffff)
		seq_no = 1;
	else if (seq_no)
		seq_no++;
	ni->mrec->sequence_number = cpu_to_le16(seq_no);

	/* Write back the record */
	if (ntfs_mft_records_write(vol, ni->mft_no, 1, ni->mrec)) {
		ntfsplus_log_error("Failed to write freed MFT record");
		return -EIO;
	}

	/* Free the inode structure */
	if (ni->mrec)
		kfree(ni->mrec);
	kfree(ni);

	ntfsplus_log_leave("Successfully freed MFT record");
	return 0;
}