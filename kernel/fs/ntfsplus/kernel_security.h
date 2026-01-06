/*
 * NTFSPLUS Kernel Module - Security Header
 * GPL Compliant - Kernel Space Adaptation
 */

#ifndef _KERNEL_NTFS_SECURITY_H
#define _KERNEL_NTFS_SECURITY_H

#include "kernel_types.h"

/* Forward declarations */
struct ntfsplus_acl;
struct ntfsplus_acl_entry;

/* Function prototypes */
int ntfsplus_security_init(struct ntfsplus_volume *vol);
void ntfsplus_security_exit(void);

int ntfsplus_get_security_context(struct inode *inode, char **context, u32 *context_len);
int ntfsplus_set_security_context(struct inode *inode, const char *context, u32 context_len);

int ntfsplus_acl_from_ntfs(const void *sd, size_t sd_len, struct ntfsplus_acl **acl);
int ntfsplus_acl_to_ntfs(struct ntfsplus_acl *acl, void *sd, size_t *sd_len);
int ntfsplus_check_acl(struct inode *inode, int mask);

void ntfsplus_audit_log(int type, struct inode *inode, const char *op, int result);

#endif /* _KERNEL_NTFS_SECURITY_H */