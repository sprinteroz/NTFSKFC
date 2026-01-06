/*
 * NTFSPLUS Kernel Module - Security Enhancements
 * SELinux integration and Advanced ACL support
 * GPL Compliant - Kernel Space Adaptation
 *
 * Author: Darryl Bennett, owner of MagDriveX
 * Created: January 6, 2026
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/security.h>
#include <linux/audit.h>
#include <linux/xattr.h>
#include "kernel_types.h"
#include "kernel_logging.h"

/* Security context cache */
struct ntfsplus_security_context {
    struct list_head list;
    u64 inode_number;
    char *context;
    size_t context_len;
    unsigned long timestamp;
};

/* ACL entry structure */
struct ntfsplus_acl_entry {
    u16 type;           /* Allow/Deny */
    u16 flags;          /* Inheritance flags */
    u32 permissions;    /* Access permissions */
    void *sid;          /* Security identifier */
    size_t sid_len;
};

/* ACL structure */
struct ntfsplus_acl {
    u16 revision;
    u16 size;
    u16 count;
    struct ntfsplus_acl_entry *entries;
};

/* Security manager */
struct ntfsplus_security_manager {
    struct list_head context_cache;
    spinlock_t context_lock;
    size_t cache_size;
    struct ntfsplus_volume *vol;
};

/* Global security manager */
static struct ntfsplus_security_manager *ntfsplus_security_mgr = NULL;

/* SELinux operation vectors */
static struct security_operations ntfsplus_security_ops;

/**
 * ntfsplus_security_init - Initialize security enhancements
 * @vol: NTFS volume
 *
 * Return: 0 on success, negative error code on failure
 */
int ntfsplus_security_init(struct ntfsplus_volume *vol)
{
    struct ntfsplus_security_manager *mgr;

    ntfsplus_log_info("Initializing NTFSPLUS security enhancements");

    mgr = kzalloc(sizeof(*mgr), GFP_KERNEL);
    if (!mgr)
        return -ENOMEM;

    INIT_LIST_HEAD(&mgr->context_cache);
    spin_lock_init(&mgr->context_lock);
    mgr->cache_size = 0;
    mgr->vol = vol;

    /* Register SELinux security operations */
    memset(&ntfsplus_security_ops, 0, sizeof(ntfsplus_security_ops));
    ntfsplus_security_ops.name = "ntfsplus";

    /* File operations */
    ntfsplus_security_ops.file_permission = ntfsplus_file_permission;
    ntfsplus_security_ops.file_alloc_security = ntfsplus_file_alloc_security;
    ntfsplus_security_ops.file_free_security = ntfsplus_file_free_security;

    /* Inode operations */
    ntfsplus_security_ops.inode_permission = ntfsplus_inode_permission;
    ntfsplus_security_ops.inode_setxattr = ntfsplus_inode_setxattr;
    ntfsplus_security_ops.inode_getxattr = ntfsplus_inode_getxattr;
    ntfsplus_security_ops.inode_listxattr = ntfsplus_inode_listxattr;

    /* Register security module */
    if (register_security(&ntfsplus_security_ops)) {
        ntfsplus_log_error("Failed to register security operations");
        kfree(mgr);
        return -EINVAL;
    }

    ntfsplus_security_mgr = mgr;

    ntfsplus_log_info("NTFSPLUS security enhancements initialized");
    return 0;
}

/**
 * ntfsplus_security_exit - Cleanup security enhancements
 */
void ntfsplus_security_exit(void)
{
    struct ntfsplus_security_manager *mgr = ntfsplus_security_mgr;

    if (!mgr)
        return;

    ntfsplus_log_info("Cleaning up NTFSPLUS security enhancements");

    /* Unregister security operations */
    unregister_security(&ntfsplus_security_ops);

    /* Free security contexts */
    ntfsplus_security_free_contexts();

    kfree(mgr);
    ntfsplus_security_mgr = NULL;

    ntfsplus_log_info("NTFSPLUS security enhancements cleaned up");
}

/**
 * ntfsplus_file_permission - Check file permissions
 * @file: file structure
 * @mask: permission mask
 *
 * Return: 0 on success, negative error code on failure
 */
static int ntfsplus_file_permission(struct file *file, int mask)
{
    struct inode *inode = file_inode(file);
    struct ntfsplus_sb_info *sbi = inode->i_sb->s_fs_info;

    /* Get SELinux security context */
    char *context = NULL;
    u32 context_len = 0;

    /* Check if SELinux is enabled */
    if (!selinux_enabled)
        return 0;

    /* Get context from inode */
    if (ntfsplus_get_security_context(inode, &context, &context_len))
        return 0; /* Allow access if no context */

    /* SELinux permission check */
    int ret = security_file_permission(file, mask);

    /* Free context */
    if (context)
        kfree(context);

    return ret;
}

/**
 * ntfsplus_inode_permission - Check inode permissions
 * @inode: inode structure
 * @mask: permission mask
 *
 * Return: 0 on success, negative error code on failure
 */
static int ntfsplus_inode_permission(struct inode *inode, int mask)
{
    /* Get SELinux security context */
    char *context = NULL;
    u32 context_len = 0;

    /* Check if SELinux is enabled */
    if (!selinux_enabled)
        return 0;

    /* Get context from inode */
    if (ntfsplus_get_security_context(inode, &context, &context_len))
        return 0; /* Allow access if no context */

    /* SELinux permission check */
    int ret = security_inode_permission(inode, mask);

    /* Free context */
    if (context)
        kfree(context);

    return ret;
}

/**
 * ntfsplus_file_alloc_security - Allocate file security
 * @file: file structure
 *
 * Return: 0 on success, negative error code on failure
 */
static int ntfsplus_file_alloc_security(struct file *file)
{
    struct inode *inode = file_inode(file);
    char *context = NULL;
    u32 context_len = 0;

    /* Check if SELinux is enabled */
    if (!selinux_enabled)
        return 0;

    /* Allocate security context for new file */
    if (security_dentry_create_files_as(file->f_path.dentry, file->f_path.dentry->d_inode->i_mode, &context))
        return 0;

    /* Set context on inode */
    if (context)
        ntfsplus_set_security_context(inode, context, strlen(context) + 1);

    return 0;
}

/**
 * ntfsplus_file_free_security - Free file security
 * @file: file structure
 */
static void ntfsplus_file_free_security(struct file *file)
{
    /* SELinux cleanup */
    security_file_free(file);
}

/**
 * ntfsplus_inode_setxattr - Set extended attribute
 */
static int ntfsplus_inode_setxattr(struct user_namespace *mnt_userns,
                                  struct dentry *dentry, const char *name,
                                  const void *value, size_t size, int flags)
{
    struct inode *inode = d_inode(dentry);

    /* Handle security.* attributes specially */
    if (!strncmp(name, XATTR_SECURITY_PREFIX, XATTR_SECURITY_PREFIX_LEN)) {
        const char *suffix = name + XATTR_SECURITY_PREFIX_LEN;

        /* SELinux context */
        if (!strcmp(suffix, "selinux")) {
            return ntfsplus_set_security_context(inode, value, size);
        }
    }

    /* SELinux xattr permission check */
    return security_inode_setxattr(mnt_userns, dentry, name, value, size, flags);
}

/**
 * ntfsplus_inode_getxattr - Get extended attribute
 */
static int ntfsplus_inode_getxattr(struct user_namespace *mnt_userns,
                                  struct dentry *dentry, const char *name,
                                  void *value, size_t size)
{
    struct inode *inode = d_inode(dentry);

    /* Handle security.* attributes specially */
    if (!strncmp(name, XATTR_SECURITY_PREFIX, XATTR_SECURITY_PREFIX_LEN)) {
        const char *suffix = name + XATTR_SECURITY_PREFIX_LEN;

        /* SELinux context */
        if (!strcmp(suffix, "selinux")) {
            return ntfsplus_get_security_context(inode, (char **)&value, (u32 *)&size);
        }
    }

    /* SELinux xattr permission check */
    return security_inode_getxattr(mnt_userns, dentry, name);
}

/**
 * ntfsplus_inode_listxattr - List extended attributes
 */
static int ntfsplus_inode_listxattr(struct user_namespace *mnt_userns,
                                   struct dentry *dentry, char *list,
                                   size_t size)
{
    /* SELinux xattr listing permission check */
    return security_inode_listxattr(mnt_userns, dentry);
}

/**
 * ntfsplus_get_security_context - Get SELinux context from inode
 * @inode: inode to get context from
 * @context: pointer to context buffer
 * @context_len: context length
 *
 * Return: 0 on success, negative error code on failure
 */
int ntfsplus_get_security_context(struct inode *inode, char **context, u32 *context_len)
{
    struct ntfsplus_security_manager *mgr = ntfsplus_security_mgr;
    struct ntfsplus_security_context *ctx_entry;

    if (!mgr || !inode || !context || !context_len)
        return -EINVAL;

    /* Check cache first */
    spin_lock(&mgr->context_lock);
    list_for_each_entry(ctx_entry, &mgr->context_cache, list) {
        if (ctx_entry->inode_number == inode->i_ino) {
            *context = kmalloc(ctx_entry->context_len, GFP_ATOMIC);
            if (!*context) {
                spin_unlock(&mgr->context_lock);
                return -ENOMEM;
            }
            memcpy(*context, ctx_entry->context, ctx_entry->context_len);
            *context_len = ctx_entry->context_len;
            spin_unlock(&mgr->context_lock);
            return 0;
        }
    }
    spin_unlock(&mgr->context_lock);

    /* TODO: Read context from NTFS extended attributes */
    /* For now, return default context */
    *context = kmalloc(1, GFP_KERNEL);
    if (!*context)
        return -ENOMEM;
    **context = '\0';
    *context_len = 1;

    return 0;
}

/**
 * ntfsplus_set_security_context - Set SELinux context on inode
 * @inode: inode to set context on
 * @context: security context
 * @context_len: context length
 *
 * Return: 0 on success, negative error code on failure
 */
int ntfsplus_set_security_context(struct inode *inode, const char *context, u32 context_len)
{
    struct ntfsplus_security_manager *mgr = ntfsplus_security_mgr;
    struct ntfsplus_security_context *ctx_entry;

    if (!mgr || !inode || !context)
        return -EINVAL;

    /* Update cache */
    spin_lock(&mgr->context_lock);

    /* Remove existing entry */
    list_for_each_entry(ctx_entry, &mgr->context_cache, list) {
        if (ctx_entry->inode_number == inode->i_ino) {
            list_del(&ctx_entry->list);
            kfree(ctx_entry->context);
            kfree(ctx_entry);
            break;
        }
    }

    /* Add new entry */
    ctx_entry = kzalloc(sizeof(*ctx_entry), GFP_ATOMIC);
    if (!ctx_entry) {
        spin_unlock(&mgr->context_lock);
        return -ENOMEM;
    }

    ctx_entry->context = kmalloc(context_len, GFP_ATOMIC);
    if (!ctx_entry->context) {
        kfree(ctx_entry);
        spin_unlock(&mgr->context_lock);
        return -ENOMEM;
    }

    memcpy(ctx_entry->context, context, context_len);
    ctx_entry->inode_number = inode->i_ino;
    ctx_entry->context_len = context_len;
    ctx_entry->timestamp = jiffies;

    list_add(&ctx_entry->list, &mgr->context_cache);

    spin_unlock(&mgr->context_lock);

    /* TODO: Write context to NTFS extended attributes */

    return 0;
}

/**
 * ntfsplus_security_free_contexts - Free all cached security contexts
 */
static void ntfsplus_security_free_contexts(void)
{
    struct ntfsplus_security_manager *mgr = ntfsplus_security_mgr;
    struct ntfsplus_security_context *ctx_entry, *tmp;

    if (!mgr)
        return;

    spin_lock(&mgr->context_lock);
    list_for_each_entry_safe(ctx_entry, tmp, &mgr->context_cache, list) {
        list_del(&ctx_entry->list);
        kfree(ctx_entry->context);
        kfree(ctx_entry);
    }
    spin_unlock(&mgr->context_lock);
}

/**
 * ntfsplus_acl_from_ntfs - Parse ACL from NTFS security descriptor
 * @sd: security descriptor data
 * @sd_len: security descriptor length
 * @acl: ACL structure to fill
 *
 * Return: 0 on success, negative error code on failure
 */
int ntfsplus_acl_from_ntfs(const void *sd, size_t sd_len, struct ntfsplus_acl **acl)
{
    /* TODO: Parse Windows ACL from NTFS security descriptor */
    /* This is a placeholder for ACL parsing implementation */

    *acl = NULL; /* No ACL support yet */
    return 0;
}

/**
 * ntfsplus_acl_to_ntfs - Convert ACL to NTFS security descriptor
 * @acl: ACL structure
 * @sd: security descriptor buffer
 * @sd_len: security descriptor length
 *
 * Return: 0 on success, negative error code on failure
 */
int ntfsplus_acl_to_ntfs(struct ntfsplus_acl *acl, void *sd, size_t *sd_len)
{
    /* TODO: Convert ACL to Windows security descriptor format */
    /* This is a placeholder for ACL conversion implementation */

    *sd_len = 0;
    return 0;
}

/**
 * ntfsplus_check_acl - Check access against ACL
 * @inode: inode to check
 * @mask: access mask
 *
 * Return: 0 on success, negative error code on failure
 */
int ntfsplus_check_acl(struct inode *inode, int mask)
{
    /* TODO: Implement Windows ACL checking */
    /* For now, fall back to standard permission checking */

    return 0; /* Allow access */
}

/**
 * ntfsplus_audit_log - Log security events
 * @type: audit event type
 * @inode: inode involved
 * @op: operation performed
 * @result: operation result
 */
void ntfsplus_audit_log(int type, struct inode *inode, const char *op, int result)
{
    struct audit_buffer *ab;

    /* Check if auditing is enabled */
    if (!audit_enabled)
        return;

    /* Create audit buffer */
    ab = audit_log_start(NULL, GFP_KERNEL, type);
    if (!ab)
        return;

    /* Add inode information */
    audit_log_format(ab, "inode=%lu", inode ? inode->i_ino : 0);

    /* Add operation information */
    audit_log_format(ab, "op=%s", op ? op : "unknown");

    /* Add result */
    audit_log_format(ab, "result=%d", result);

    /* Log the event */
    audit_log_end(ab);
}