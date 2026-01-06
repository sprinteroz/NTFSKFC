/*
 * NTFSPLUS Kernel Module - Volume Management
 * Converted from ntfs-3g volume.c to kernel space
 * GPL Compliant - Kernel Space Adaptation
 *
 * Author: Darryl Bennett, owner of MagDriveX
 * Created: January 6, 2026
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/buffer_head.h>
#include <linux/blkdev.h>
#include <linux/vfs.h>
#include "kernel_types.h"

/* Volume flags - kernel compatible */
#define NTFSPLUS_VOL_READONLY     0x0001
#define NTFSPLUS_VOL_MOUNTED      0x0002
#define NTFSPLUS_VOL_DIRTY        0x0004

/* Volume flags - kernel compatible */
#define NTFSPLUS_VOL_READONLY     0x0001
#define NTFSPLUS_VOL_MOUNTED      0x0002
#define NTFSPLUS_VOL_DIRTY        0x0004

/* Forward declarations */
static int ntfsplus_read_boot_sector(struct ntfsplus_volume *vol);
static int ntfsplus_setup_mft(struct ntfsplus_volume *vol);

/*
 * ntfsplus_volume_alloc - Allocate kernel NTFS volume structure
 */
struct ntfsplus_volume *ntfsplus_volume_alloc(void)
{
    struct ntfsplus_volume *vol;

    vol = kzalloc(sizeof(*vol), GFP_KERNEL);
    if (!vol)
        return ERR_PTR(-ENOMEM);

    mutex_init(&vol->volume_mutex);
    return vol;
}

/*
 * ntfsplus_volume_free - Free kernel NTFS volume structure
 */
void ntfsplus_volume_free(struct ntfsplus_volume *vol)
{
    if (!vol)
        return;

    if (vol->mft_cache)
        kmem_cache_destroy(vol->mft_cache);

    if (vol->upcase)
        kfree(vol->upcase);

    kfree(vol);
}

/*
 * ntfsplus_read_boot_sector - Read and parse NTFS boot sector
 * Adapted from ntfs_boot_sector_parse in ntfs-3g
 */
static int ntfsplus_read_boot_sector(struct ntfsplus_volume *vol)
{
    struct buffer_head *bh;
    NTFS_BOOT_SECTOR *bs;
    u32 sector_size;
    int ret = -EINVAL;

    /* Read sector 0 */
    bh = sb_bread(vol->sb, 0);
    if (!bh) {
        pr_err("NTFSPLUS: Failed to read boot sector\n");
        return -EIO;
    }

    bs = (NTFS_BOOT_SECTOR *)bh->b_data;

    /* Basic validation */
    if (bs->jump[0] != 0xeb || bs->oem_id[0] != 'N' ||
        bs->oem_id[1] != 'T' || bs->oem_id[2] != 'F' ||
        bs->oem_id[3] != 'S') {
        pr_err("NTFSPLUS: Invalid boot sector signature\n");
        goto out;
    }

    sector_size = le16_to_cpu(bs->bytes_per_sector);
    if (sector_size != 512 && sector_size != 1024 &&
        sector_size != 2048 && sector_size != 4096) {
        pr_err("NTFSPLUS: Invalid sector size %u\n", sector_size);
        goto out;
    }

    /* Parse boot sector - simplified for now */
    vol->cluster_size = sector_size * bs->sectors_per_cluster;
    /* Use a reasonable default for testing - will be fixed in Phase 2 */
    vol->nr_clusters = 1024 * 1024;  /* 1GB default */
    vol->mft_lcn = sle64_to_cpu(bs->mft_lcn);
    vol->mftmirr_lcn = sle64_to_cpu(bs->mftmirr_lcn);

    /* Calculate MFT record size */
    if (bs->clusters_per_mft_record > 0)
        vol->mft_record_size = bs->clusters_per_mft_record *
                              vol->cluster_size;
    else
        vol->mft_record_size = 1 << (0 - bs->clusters_per_mft_record);

    pr_info("NTFSPLUS: Volume initialized - %llu clusters, %u cluster size\n",
            vol->nr_clusters, vol->cluster_size);

    ret = 0;

out:
    brelse(bh);
    return ret;
}

/*
 * ntfsplus_setup_mft - Setup MFT access structures
 */
static int ntfsplus_setup_mft(struct ntfsplus_volume *vol)
{
    /* Create MFT record cache */
    vol->mft_cache = kmem_cache_create("ntfsplus_mft",
                                      vol->mft_record_size,
                                      0, SLAB_HWCACHE_ALIGN, NULL);
    if (!vol->mft_cache) {
        pr_err("NTFSPLUS: Failed to create MFT cache\n");
        return -ENOMEM;
    }

    return 0;
}

/*
 * ntfsplus_volume_startup - Initialize NTFS volume from superblock
 * Kernel equivalent of ntfs_volume_startup
 */
int ntfsplus_volume_startup(struct super_block *sb, struct ntfsplus_volume **vol_out)
{
    struct ntfsplus_volume *vol;
    int ret;

    pr_info("NTFSPLUS: Starting volume initialization\n");

    vol = ntfsplus_volume_alloc();
    if (IS_ERR(vol))
        return PTR_ERR(vol);

    vol->sb = sb;
    vol->bdev = sb->s_bdev;

    /* Read and validate boot sector */
    ret = ntfsplus_read_boot_sector(vol);
    if (ret)
        goto err_free_vol;

    /* Setup MFT structures */
    ret = ntfsplus_setup_mft(vol);
    if (ret)
        goto err_free_vol;

    /* Setup default upcase table */
    vol->upcase_len = 65536;
    vol->upcase = kmalloc(vol->upcase_len * sizeof(ntfschar), GFP_KERNEL);
    if (!vol->upcase) {
        ret = -ENOMEM;
        goto err_free_vol;
    }

    /* Initialize upcase table (simplified) */
    for (u32 i = 0; i < vol->upcase_len; i++) {
        if (i >= 'a' && i <= 'z')
            vol->upcase[i] = cpu_to_le16(i - 'a' + 'A');
        else
            vol->upcase[i] = cpu_to_le16(i);
    }

    *vol_out = vol;
    pr_info("NTFSPLUS: Volume startup complete\n");
    return 0;

err_free_vol:
    ntfsplus_volume_free(vol);
    return ret;
}

/*
 * ntfsplus_volume_shutdown - Shutdown NTFS volume
 */
void ntfsplus_volume_shutdown(struct ntfsplus_volume *vol)
{
    if (!vol)
        return;

    pr_info("NTFSPLUS: Shutting down volume\n");
    ntfsplus_volume_free(vol);
}