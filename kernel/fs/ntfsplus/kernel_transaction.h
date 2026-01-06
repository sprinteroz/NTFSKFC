/*
 * NTFSPLUS Kernel Module - Transaction Header
 * GPL Compliant - Kernel Space Adaptation
 */

#ifndef _KERNEL_NTFS_TRANSACTION_H
#define _KERNEL_NTFS_TRANSACTION_H

#include "kernel_types.h"

/* Forward declarations */
struct ntfsplus_volume;
struct ntfsplus_transaction;

/* Transaction types */
#define NTFS_TRANSACTION_FILE        0x01
#define NTFS_TRANSACTION_METADATA    0x02
#define NTFS_TRANSACTION_LOG         0x04

/* Transaction states */
#define NTFS_TRANSACTION_ACTIVE      0x01
#define NTFS_TRANSACTION_COMMITTED   0x02
#define NTFS_TRANSACTION_ABORTED     0x04
#define NTFS_TRANSACTION_PREPARED    0x08

/* Operation types */
#define NTFS_OP_CREATE_FILE          0x01
#define NTFS_OP_DELETE_FILE          0x02
#define NTFS_OP_WRITE_DATA           0x03
#define NTFS_OP_UPDATE_METADATA      0x04
#define NTFS_OP_CREATE_DIR           0x05
#define NTFS_OP_DELETE_DIR           0x06

/* Function prototypes */
int ntfsplus_transaction_init(struct ntfsplus_volume *vol);
void ntfsplus_transaction_exit(void);

struct ntfsplus_transaction *ntfsplus_transaction_begin(u32 type);
int ntfsplus_transaction_add_operation(struct ntfsplus_transaction *txn,
                                      u32 op_type, void *data, size_t data_size,
                                      void (*undo_func)(void *),
                                      void (*redo_func)(void *));
int ntfsplus_transaction_commit(struct ntfsplus_transaction *txn);
int ntfsplus_transaction_rollback(struct ntfsplus_transaction *txn);

u32 ntfsplus_transaction_get_state(struct ntfsplus_transaction *txn);
int ntfsplus_transaction_is_active(struct ntfsplus_transaction *txn);
u64 ntfsplus_transaction_get_id(struct ntfsplus_transaction *txn);

#endif /* _KERNEL_NTFS_TRANSACTION_H */