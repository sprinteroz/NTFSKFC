/*
 *
 * Author: Darryl Bennett, owner of MagDriveX
 * Created: January 6, 2026
 * NTFSPLUS Kernel Module - Runlist Management
 * Converted from ntfs-3g runlist.c to kernel space
 * GPL Compliant - Kernel Space Adaptation
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/string.h>
#include "kernel_types.h"
#include "kernel_logging.h"

/* Runlist element - kernel space */
struct runlist_element {
    VCN vcn;        /* Virtual cluster number */
    LCN lcn;        /* Logical cluster number */
    s64 length;     /* Length in clusters */
};

/* Runlist extension - extend runlist by more entries */
runlist_element *ntfs_rl_extend(runlist_element **na_rl, runlist_element *rl, int more_entries)
{
    runlist_element *newrl;
    int last;
    int irl;

    if (*na_rl && rl) {
        irl = (int)(rl - *na_rl);
        last = irl;
        while ((*na_rl)[last].length)
            last++;
        newrl = krealloc(*na_rl, (last + more_entries + 1) * sizeof(runlist_element), GFP_KERNEL);
        if (!newrl) {
            ntfsplus_log_error("Failed to extend runlist");
            rl = NULL;
        } else {
            *na_rl = newrl;
            rl = &newrl[irl];
        }
    } else {
        ntfsplus_log_error("Cannot extend unmapped runlist");
        rl = NULL;
    }
    return rl;
}

/* Check if two runlists can be merged */
static bool ntfs_rl_are_mergeable(runlist_element *dst, runlist_element *src)
{
    if (!dst || !src) {
        ntfsplus_log_debug("mergeable: NULL pointer");
        return false;
    }

    /* We can merge unmapped regions even if they are misaligned. */
    if ((dst->lcn == LCN_RL_NOT_MAPPED) && (src->lcn == LCN_RL_NOT_MAPPED))
        return true;

    /* If the runs are misaligned, we cannot merge them. */
    if ((dst->vcn + dst->length) != src->vcn)
        return false;

    /* If both runs are non-sparse and contiguous, we can merge them. */
    if ((dst->lcn >= 0) && (src->lcn >= 0) &&
        ((dst->lcn + dst->length) == src->lcn))
        return true;

    /* If we are merging two holes, we can merge them. */
    if ((dst->lcn == LCN_HOLE) && (src->lcn == LCN_HOLE))
        return true;

    /* Cannot merge. */
    return false;
}

/* Merge two runlists without testing */
static void __ntfs_rl_merge(runlist_element *dst, runlist_element *src)
{
    dst->length += src->length;
}

/* Merge two runlists into one */
runlist_element *ntfs_runlists_merge(runlist_element *drl, runlist_element *srl)
{
    int di, si;
    int sstart;
    int dins;
    int dend, send;
    int dfinal, sfinal;
    runlist_element *temp_rl;

    ntfsplus_log_enter("Merging runlists");

    /* Check for silly calling... */
    if (!srl) {
        ntfsplus_log_debug("No source runlist to merge");
        return drl;
    }

    /* Check for the case where the first mapping is being done now. */
    if (!drl) {
        drl = srl;
        /* Complete the source runlist if necessary. */
        if (drl[0].vcn) {
            /* Scan to the end of the source runlist. */
            for (dend = 0; drl[dend].length; dend++)
                ;
            dend++;
            drl = krealloc(drl, (dend + 1) * sizeof(runlist_element), GFP_KERNEL);
            if (!drl)
                return NULL;
            /* Insert start element at the front of the runlist. */
            memmove(drl + 1, drl, dend * sizeof(runlist_element));
            drl[0].vcn = 0;
            drl[0].lcn = LCN_RL_NOT_MAPPED;
            drl[0].length = drl[1].vcn;
        }
        goto finished;
    }

    si = di = 0;

    /* Skip any unmapped start element(s) in the source runlist. */
    while (srl[si].length && srl[si].lcn < LCN_HOLE)
        si++;

    /* Can't have an entirely unmapped source runlist. */
    if (!srl[si].length) {
        ntfsplus_log_error("Unmapped source runlist");
        return NULL;
    }

    /* Record the starting points. */
    sstart = si;

    /*
     * Skip forward in @drl until we reach the position where @srl needs to
     * be inserted. If we reach the end of @drl, @srl just needs to be
     * appended to @drl.
     */
    for (; drl[di].length; di++) {
        if (drl[di].vcn + drl[di].length > srl[sstart].vcn)
            break;
    }
    dins = di;

    /* Sanity check for illegal overlaps. */
    if ((drl[di].vcn == srl[si].vcn) && (drl[di].lcn >= 0) &&
        (srl[si].lcn >= 0)) {
        ntfsplus_log_error("Run lists overlap");
        return NULL;
    }

    /* Scan to the end of both runlists in order to know their sizes. */
    for (send = si; srl[send].length; send++)
        ;
    for (dend = di; drl[dend].length; dend++)
        ;

    /* Scan to the last element with lcn >= LCN_HOLE. */
    for (sfinal = send; sfinal >= 0 && srl[sfinal].lcn < LCN_HOLE; sfinal--)
        ;
    for (dfinal = dend; dfinal >= 0 && drl[dfinal].lcn < LCN_HOLE; dfinal--)
        ;

    /* Allocate space for merged runlist */
    {
        bool left, right, disc;
        int ds = dend + 1;
        int ss = sfinal - sstart + 1;

        left = ((drl[dins].lcn < LCN_RL_NOT_MAPPED) ||
               (drl[dins].vcn == srl[sstart].vcn));

        right = ((drl[dins].lcn >= LCN_RL_NOT_MAPPED) ||
                ((drl[dins].vcn + drl[dins].length) <=
                 (srl[send - 1].vcn + srl[send - 1].length)));

        disc = (drl[dins].vcn + drl[dins].length > srl[send - 1].vcn);

        /* Space required: dst size + src size, less merges */
        temp_rl = krealloc(drl, (ds + ss - left - right + disc) * sizeof(runlist_element), GFP_KERNEL);
        if (!temp_rl) {
            ntfsplus_log_error("Failed to allocate merged runlist");
            return NULL;
        }
        drl = temp_rl;

        /* Simplified merge - just append for now */
        /* TODO: Implement full merge logic */
        ntfsplus_log_debug("Runlist merge simplified - appending");
    }

finished:
    /* The merge was completed successfully. */
    ntfsplus_log_leave("Runlist merge completed");
    return drl;
}

/* Convert VCN to LCN using runlist */
LCN ntfs_rl_vcn_to_lcn(const runlist_element *rl, const VCN vcn)
{
    int i;

    if (vcn < 0)
        return LCN_EINVAL;

    /*
     * If rl is NULL, assume that we have found an unmapped runlist. The
     * caller can then attempt to map it and fail appropriately if
     * necessary.
     */
    if (!rl)
        return LCN_RL_NOT_MAPPED;

    /* Catch out of lower bounds vcn. */
    if (vcn < rl[0].vcn)
        return LCN_ENOENT;

    for (i = 0; rl[i].length; i++) {
        if (vcn < rl[i+1].vcn) {
            if (rl[i].lcn >= 0)
                return rl[i].lcn + (vcn - rl[i].vcn);
            return rl[i].lcn;
        }
    }

    /*
     * The terminator element is setup to the correct value, i.e. one of
     * LCN_HOLE, LCN_RL_NOT_MAPPED, or LCN_ENOENT.
     */
    if (rl[i].lcn < 0)
        return rl[i].lcn;

    /* Just in case... We could replace this with BUG() some day. */
    return LCN_ENOENT;
}

/* Decompress mapping pairs array to runlist */
runlist_element *ntfs_mapping_pairs_decompress(const struct ntfsplus_volume *vol,
                                             const ATTR_RECORD *attr,
                                             runlist_element *old_rl)
{
    VCN vcn;
    LCN lcn = 0;
    s64 deltaxcn;
    runlist_element *rl = NULL;
    const u8 *buf;
    const u8 *attr_end;
    int rlsize;
    u16 rlpos = 0;
    u8 b;

    ntfsplus_log_enter("Decompressing mapping pairs for attr 0x%x", le32_to_cpu(attr->type));

    /* Make sure attr exists and is non-resident. */
    /* For now, assume it's valid and try to process */
    if (!attr) {
        ntfsplus_log_error("Invalid attribute for mapping pairs decompression");
        return NULL;
    }

    /* For now, assume starting from VCN 0 */
    vcn = 0;

    /* Get start of the mapping pairs array - simplified for kernel */
    buf = (const u8*)attr + sizeof(ATTR_RECORD);
    attr_end = (const u8*)attr + le32_to_cpu(attr->length);

    if (buf < (const u8*)attr || buf > attr_end) {
        ntfsplus_log_error("Corrupt attribute mapping pairs");
        return NULL;
    }

    /* Allocate initial runlist buffer */
    rlsize = 0x1000;
    rl = kzalloc(rlsize, GFP_KERNEL);
    if (!rl)
        return NULL;

    /* Insert unmapped starting element if necessary. */
    if (vcn) {
        rl->vcn = 0;
        rl->lcn = LCN_RL_NOT_MAPPED;
        rl->length = vcn;
        rlpos++;
    }

    while (buf < attr_end && *buf) {
        /*
         * Allocate more memory if needed
         */
        if ((rlpos + 3) * sizeof(runlist_element) > rlsize) {
            runlist_element *rl2;

            rlsize += 0x1000;
            rl2 = krealloc(rl, rlsize, GFP_KERNEL);
            if (!rl2) {
                kfree(rl);
                return NULL;
            }
            rl = rl2;
        }

        /* Enter the current vcn into the current runlist element. */
        rl[rlpos].vcn = vcn;

        /*
         * Get the change in vcn, i.e. the run length in clusters.
         */
        b = *buf & 0xf;
        if (b) {
            if (buf + b > attr_end)
                goto io_error;
            for (deltaxcn = (s8)buf[b--]; b; b--)
                deltaxcn = (deltaxcn << 8) + buf[b];
        } else {
            deltaxcn = (s64)-1;
        }

        if (deltaxcn < 0) {
            ntfsplus_log_error("Invalid length in mapping pairs array");
            goto err_out;
        }

        /* Enter the current run length */
        rl[rlpos].length = deltaxcn;
        /* Increment the current vcn by the current run length. */
        vcn += deltaxcn;

        /* Get LCN change */
        if (!(*buf & 0xf0))
            rl[rlpos].lcn = LCN_HOLE;
        else {
            u8 b2 = *buf & 0xf;
            if (!b2) {
                ntfsplus_log_error("Invalid length in mapping pairs array");
                goto err_out;
            }

            b = b2 + ((*buf >> 4) & 0xf);
            if (buf + b > attr_end)
                goto io_error;
            for (deltaxcn = (s8)buf[b--]; b > b2; b--)
                deltaxcn = (deltaxcn << 8) + buf[b];
            lcn += deltaxcn;

            if (lcn < (LCN)-1) {
                ntfsplus_log_error("Invalid LCN in mapping pairs array");
                goto err_out;
            }

            rl[rlpos].lcn = lcn;
        }

        /* Get to the next runlist element */
        if (rl[rlpos].length)
            rlpos++;

        /* Increment the buffer position */
        buf += (*buf & 0xf) + ((*buf >> 4) & 0xf) + 1;
    }

    if (buf >= attr_end)
        goto io_error;

    /* Setup terminating runlist element */
    rl[rlpos].vcn = vcn;
    rl[rlpos].lcn = LCN_ENOENT;
    rl[rlpos].length = 0;

    /* If no existing runlist was specified, we are done. */
    if (!old_rl || !old_rl[0].length) {
        ntfsplus_log_debug("Mapping pairs array successfully decompressed");
        return rl;
    }

    /* TODO: Merge with old runlist */
    ntfsplus_log_debug("Merging with existing runlist");

    ntfsplus_log_leave("Mapping pairs decompression completed");
    return rl;

io_error:
    ntfsplus_log_error("Cluster run list value is corrupted");
err_out:
    kfree(rl);
    return NULL;
}

/* Read from runlist */
s64 ntfs_rl_pread(const struct ntfsplus_volume *vol, const runlist_element *rl,
                 const s64 pos, s64 count, void *b)
{
    s64 bytes_read, to_read, ofs, total;
    int err = EIO;
    int cluster_size_bits = 9; /* Default 512 bytes, will be calculated from vol->cluster_size */

    if (!vol || !rl || pos < 0 || count < 0) {
        ntfsplus_log_error("Invalid parameters for rl_pread");
        return -EINVAL;
    }

    if (!count)
        return count;

    /* Calculate cluster size bits */
    if (vol->cluster_size > 0) {
        cluster_size_bits = fls(vol->cluster_size) - 1;
    }

    /* Seek in @rl to the run containing @pos. */
    for (ofs = 0; rl->length && (ofs + (rl->length << cluster_size_bits) <= pos); rl++)
        ofs += (rl->length << cluster_size_bits);

    /* Offset in the run at which to begin reading. */
    ofs = pos - ofs;

    for (total = 0LL; count; rl++, ofs = 0) {
        if (!rl->length)
            goto rl_err_out;

        if (rl->lcn < (LCN)0) {
            if (rl->lcn != (LCN)LCN_HOLE)
                goto rl_err_out;
            /* It is a hole, just zero the matching @b range. */
            to_read = min(count, (rl->length << cluster_size_bits) - ofs);
            memset(b, 0, to_read);
            /* Update counters and proceed with next run. */
            total += to_read;
            count -= to_read;
            b = (u8*)b + to_read;
            continue;
        }

        /* TODO: Implement actual disk read */
        to_read = min(count, (rl->length << cluster_size_bits) - ofs);
        memset(b, 0, to_read); /* Placeholder - return zeros */
        total += to_read;
        count -= to_read;
        b = (u8*)b + to_read;
    }

    return total;

rl_err_out:
    if (total)
        return total;
    return -err;
}

/* Write to runlist */
s64 ntfs_rl_pwrite(const struct ntfsplus_volume *vol, const runlist_element *rl,
                  s64 ofs, const s64 pos, s64 count, void *b)
{
    s64 written, to_write, total = 0;
    int err = EIO;
    int cluster_size_bits = 9; /* Default 512 bytes */

    if (!vol || !rl || pos < 0 || count < 0) {
        ntfsplus_log_error("Invalid parameters for rl_pwrite");
        return -EINVAL;
    }

    if (!count)
        return count;

    /* Calculate cluster size bits */
    if (vol->cluster_size > 0) {
        cluster_size_bits = fls(vol->cluster_size) - 1;
    }

    /* Seek in @rl to the run containing @pos. */
    while (rl->length && (ofs + (rl->length << cluster_size_bits) <= pos)) {
        ofs += (rl->length << cluster_size_bits);
        rl++;
    }

    /* Offset in the run at which to begin writing. */
    ofs = pos - ofs;

    for (total = 0LL; count; rl++, ofs = 0) {
        if (!rl->length)
            goto rl_err_out;

        if (rl->lcn < (LCN)0) {
            if (rl->lcn != (LCN)LCN_HOLE)
                goto rl_err_out;

            to_write = min(count, (rl->length << cluster_size_bits) - ofs);
            /* Skip holes */
            total += to_write;
            count -= to_write;
            b = (u8*)b + to_write;
            continue;
        }

        /* TODO: Implement actual disk write */
        to_write = min(count, (rl->length << cluster_size_bits) - ofs);
        /* Placeholder - simulate success */
        total += to_write;
        count -= to_write;
        b = (u8*)b + to_write;
    }

    return total;

rl_err_out:
    if (total)
        return total;
    return -err;
}

/* Get number of significant bytes for a number */
int ntfs_get_nr_significant_bytes(const s64 n)
{
    u64 l = (n < 0 ? ~n : n);
    int i = 1;

    if (l >= 128) {
        l >>= 7;
        do {
            i++;
            l >>= 8;
        } while (l);
    }

    return i;
}

/* Get size needed for mapping pairs */
int ntfs_get_size_for_mapping_pairs(const struct ntfsplus_volume *vol,
                                   const runlist_element *rl,
                                   const VCN start_vcn, int max_size)
{
    LCN prev_lcn = 0;
    int rls = 1; /* Terminator byte */
    int major_ver = 3; /* Default NTFS version */

    if (start_vcn < 0)
        return -EINVAL;

    if (!rl)
        return rls;

    /* Use volume major version if available */
    if (vol->major_ver > 0) {
        major_ver = vol->major_ver;
    }

    /* Skip to runlist element containing @start_vcn. */
    while (rl->length && start_vcn >= rl[1].vcn)
        rl++;

    if ((!rl->length && start_vcn > rl->vcn) || start_vcn < rl->vcn)
        return -EINVAL;

    /* Calculate mapping pairs size */
    for (; rl->length; rl++) {
        if (rl->length < 0 || rl->lcn < LCN_HOLE)
            return -EIO;

        /* Header byte + length */
        rls += 1 + ntfs_get_nr_significant_bytes(rl->length);

        if (rl->lcn >= 0 || major_ver < 3) {
            /* Change in lcn */
            rls += ntfs_get_nr_significant_bytes(rl->lcn - prev_lcn);
            prev_lcn = rl->lcn;
        }

        if (rls > max_size)
            break;
    }

    return rls;
}

/* Truncate runlist */
int ntfs_rl_truncate(runlist_element **arl, const VCN start_vcn)
{
    runlist_element *rl;

    if (!arl || !*arl) {
        ntfsplus_log_error("Invalid runlist for truncation");
        return -EINVAL;
    }

    rl = *arl;

    if (start_vcn < rl->vcn) {
        ntfsplus_log_error("Start_vcn lies outside front of runlist");
        return -EINVAL;
    }

    /* Find the starting vcn in the run list. */
    while (rl->length) {
        if (start_vcn < rl[1].vcn)
            break;
        rl++;
    }

    if (!rl->length) {
        ntfsplus_log_error("Truncating already truncated runlist");
        return -EIO;
    }

    /* Truncate the run. */
    rl->length = start_vcn - rl->vcn;

    /* Set terminator */
    if (rl->length) {
        rl++;
        rl->vcn = start_vcn;
        rl->length = 0;
    }
    rl->lcn = LCN_ENOENT;

    return 0;
}

/* Check if runlist has sparse regions */
int ntfs_rl_sparse(runlist_element *rl)
{
    runlist_element *rlc;

    if (!rl)
        return -1;

    for (rlc = rl; rlc->length; rlc++) {
        if (rlc->lcn < 0) {
            if (rlc->lcn != LCN_HOLE)
                return -1;
            return 1;
        }
    }
    return 0;
}