/*
 * NTFSPLUS Kernel Module - NTFS Compression Header
 * GPL Compliant - Kernel Space Adaptation
 */

#ifndef _KERNEL_NTFS_COMPRESSION_H
#define _KERNEL_NTFS_COMPRESSION_H

#include "kernel_types.h"

/* Compression types */
#define NTFS_COMPRESSION_NONE     0x0000
#define NTFS_COMPRESSION_LZNT1    0x0001
#define NTFS_COMPRESSION_LZX      0x0002

/* Function prototypes */
int ntfsplus_compression_init(void);
void ntfsplus_compression_exit(void);

int ntfsplus_lznt1_compress(const u8 *input, size_t input_size,
                           u8 *output, size_t output_size,
                           size_t *compressed_size);

int ntfsplus_lznt1_decompress(const u8 *input, size_t input_size,
                             u8 *output, size_t output_size,
                             size_t *decompressed_size);

int ntfsplus_compress_chunk(const u8 *input, u8 *output, size_t *compressed_size);
int ntfsplus_decompress_chunk(const u8 *input, size_t input_size, u8 *output);

int ntfsplus_is_compressed(const u8 *data, size_t size);
int ntfsplus_compression_ratio(size_t original_size, size_t compressed_size);
int ntfsplus_compression_supported(u16 compression_type);

#endif /* _KERNEL_NTFS_COMPRESSION_H */