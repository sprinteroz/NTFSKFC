/*
 *
 * Author: Darryl Bennett, owner of MagDriveX
 * Created: January 6, 2026
 * NTFSPLUS Kernel Module - Utility Functions
 * Converted from ntfs-3g lib_utils.c and misc.c to kernel space
 * GPL Compliant - Kernel Space Adaptation
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/time.h>
#include <linux/err.h>
#include "kernel_types.h"
#include "kernel_logging.h"

/* Memory management - kernel equivalents */

/*
 * ntfsplus_malloc - Kernel equivalent of malloc
 * Uses kmalloc with GFP_KERNEL
 */
void *ntfsplus_malloc(size_t size)
{
    void *ptr;

    if (!size)
        return NULL;

    ptr = kmalloc(size, GFP_KERNEL);
    if (!ptr) {
        ntfsplus_log_error("Failed to allocate %zu bytes\n", size);
        return NULL;
    }

    return ptr;
}

/*
 * ntfsplus_calloc - Kernel equivalent of calloc
 */
void *ntfsplus_calloc(size_t nmemb, size_t size)
{
    void *ptr;
    size_t total = nmemb * size;

    if (!total)
        return NULL;

    ptr = kzalloc(total, GFP_KERNEL);
    if (!ptr) {
        ntfsplus_log_error("Failed to calloc %zu bytes\n", total);
        return NULL;
    }

    return ptr;
}

/*
 * ntfsplus_free - Kernel equivalent of free
 */
void ntfsplus_free(void *ptr)
{
    if (ptr)
        kfree(ptr);
}

/*
 * ntfsplus_realloc - Kernel equivalent of realloc
 */
void *ntfsplus_realloc(void *ptr, size_t size)
{
    void *new_ptr;

    if (!size) {
        ntfsplus_free(ptr);
        return NULL;
    }

    if (!ptr)
        return ntfsplus_malloc(size);

    new_ptr = krealloc(ptr, size, GFP_KERNEL);
    if (!new_ptr) {
        ntfsplus_log_error("Failed to realloc to %zu bytes\n", size);
        return NULL;
    }

    return new_ptr;
}

/* String utilities - kernel equivalents */

/*
 * ntfsplus_strdup - Kernel equivalent of strdup
 */
char *ntfsplus_strdup(const char *s)
{
    size_t len;
    char *dup;

    if (!s)
        return NULL;

    len = strlen(s) + 1;
    dup = ntfsplus_malloc(len);
    if (dup)
        memcpy(dup, s, len);

    return dup;
}

/*
 * ntfsplus_strndup - Kernel equivalent of strndup
 */
char *ntfsplus_strndup(const char *s, size_t n)
{
    size_t len;
    char *dup;

    if (!s)
        return NULL;

    len = strnlen(s, n);
    dup = ntfsplus_malloc(len + 1);
    if (dup) {
        memcpy(dup, s, len);
        dup[len] = '\0';
    }

    return dup;
}

/* Unicode string handling */

/*
 * ntfsplus_ucsnlen - Get length of null-terminated Unicode string
 * Kernel equivalent of ntfs_ucsnlen
 */
int ntfsplus_ucsnlen(const ntfschar *s, int maxlen)
{
    int i;

    if (!s)
        return 0;

    for (i = 0; i < maxlen; i++) {
        if (!le16_to_cpu(s[i]))
            break;
    }

    return i;
}

/*
 * ntfsplus_ucsndup - Duplicate Unicode string
 * Kernel equivalent of ntfs_ucsndup
 */
ntfschar *ntfsplus_ucsndup(const ntfschar *s, int len)
{
    ntfschar *dup;

    if (!s || len < 0)
        return NULL;

    dup = ntfsplus_malloc((len + 1) * sizeof(ntfschar));
    if (dup) {
        memcpy(dup, s, len * sizeof(ntfschar));
        dup[len] = const_cpu_to_le16(0);
    }

    return dup;
}

/*
 * ntfsplus_ucstombs - Convert Unicode to multibyte string
 * Kernel equivalent of ntfs_ucstombs
 */
int ntfsplus_ucstombs(const ntfschar *ins, const int ins_len, char **outs, int outs_len)
{
    int i, len;
    char *out;

    if (!ins || !outs)
        return -1;

    /* Calculate required length */
    len = 0;
    for (i = 0; i < ins_len && ins[i]; i++) {
        ntfschar uc = le16_to_cpu(ins[i]);
        if (uc < 0x80)
            len += 1;
        else if (uc < 0x800)
            len += 2;
        else
            len += 3;
    }

    if (outs_len > 0 && len >= outs_len)
        return -1;

    out = ntfsplus_malloc(len + 1);
    if (!out)
        return -1;

    /* Convert Unicode to UTF-8 */
    len = 0;
    for (i = 0; i < ins_len && ins[i]; i++) {
        ntfschar uc = le16_to_cpu(ins[i]);

        if (uc < 0x80) {
            out[len++] = (char)uc;
        } else if (uc < 0x800) {
            out[len++] = (char)(0xc0 | (uc >> 6));
            out[len++] = (char)(0x80 | (uc & 0x3f));
        } else {
            out[len++] = (char)(0xe0 | (uc >> 12));
            out[len++] = (char)(0x80 | ((uc >> 6) & 0x3f));
            out[len++] = (char)(0x80 | (uc & 0x3f));
        }
    }
    out[len] = '\0';

    *outs = out;
    return len;
}

/* Time utilities */

/*
 * ntfsplus_ftime - Get current time in NTFS format
 * Kernel equivalent of time functions
 */
u64 ntfsplus_ftime(void)
{
    struct timespec64 now;
    u64 ntfs_time;

    ktime_get_real_ts64(&now);

    /* Convert Unix time to NTFS time (100ns intervals since 1601-01-01) */
    ntfs_time = (u64)now.tv_sec * 10000000ULL + now.tv_nsec / 100;
    ntfs_time += 116444736000000000ULL; /* Unix epoch to NTFS epoch */

    return cpu_to_le64(ntfs_time);
}

/* Error handling */

/*
 * Kernel doesn't use errno like userspace
 * These functions provide compatibility
 */
static int ntfsplus_errno;

int ntfsplus_get_errno(void)
{
    return ntfsplus_errno;
}

void ntfsplus_set_errno(int err)
{
    ntfsplus_errno = err;
}

/* Attribute name handling */

/*
 * ntfsplus_attr_name_free - Free attribute name
 */
void ntfsplus_attr_name_free(char **name)
{
    if (name && *name) {
        ntfsplus_free(*name);
        *name = NULL;
    }
}

/*
 * ntfsplus_attr_name_get - Get attribute name from Unicode
 */
char *ntfsplus_attr_name_get(const ntfschar *uname, const int uname_len)
{
    char *name = NULL;
    int name_len;

    name_len = ntfsplus_ucstombs(uname, uname_len, &name, 0);
    if (name_len < 0) {
        ntfsplus_log_perror("ntfsplus_ucstombs");
        return NULL;
    } else if (name_len > 0)
        return name;

    ntfsplus_attr_name_free(&name);
    return NULL;
}

/* Memory alignment utilities */

/*
 * ntfsplus_align - Align size to specified boundary
 */
size_t ntfsplus_align(size_t size, size_t alignment)
{
    return (size + alignment - 1) & ~(alignment - 1);
}

/*
 * ntfsplus_is_power_of_two - Check if number is power of 2
 */
bool ntfsplus_is_power_of_two(unsigned long n)
{
    return n && (n & (n - 1)) == 0;
}

/* Compatibility macros for gradual conversion */
#define ntfs_malloc ntfsplus_malloc
#define ntfs_calloc ntfsplus_calloc
#define ntfs_free ntfsplus_free
#define ntfs_realloc ntfsplus_realloc
#define ntfs_strdup ntfsplus_strdup
#define ntfs_strndup ntfsplus_strndup
#define ntfs_ucsnlen ntfsplus_ucsnlen
#define ntfs_ucsndup ntfsplus_ucsndup
#define ntfs_ucstombs ntfsplus_ucstombs
#define ntfs_attr_name_free ntfsplus_attr_name_free
#define ntfs_attr_name_get ntfsplus_attr_name_get