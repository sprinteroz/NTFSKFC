/*
 * NTFSPLUS Kernel Module - Logging Header
 * Function declarations for logging system
 * GPL Compliant - Kernel Space Adaptation
 */

#ifndef _KERNEL_NTFS_LOGGING_H
#define _KERNEL_NTFS_LOGGING_H

/* Logging levels */
#define NTFSPLUS_LOG_LEVEL_ERROR     0
#define NTFSPLUS_LOG_LEVEL_WARNING   1
#define NTFSPLUS_LOG_LEVEL_INFO      2
#define NTFSPLUS_LOG_LEVEL_DEBUG     3
#define NTFSPLUS_LOG_LEVEL_TRACE     4

/* Function declarations */
void ntfsplus_log_error(const char *format, ...);
void ntfsplus_log_warning(const char *format, ...);
void ntfsplus_log_info(const char *format, ...);
void ntfsplus_log_debug(const char *format, ...);
void ntfsplus_log_trace(const char *format, ...);
void ntfsplus_log_enter(const char *format, ...);
void ntfsplus_log_leave(const char *format, ...);
void ntfsplus_log_perror(const char *format, ...);
void ntfsplus_log_verbose(const char *format, ...);

void ntfsplus_set_log_level(int level);
int ntfsplus_get_log_level(void);

#endif /* _KERNEL_NTFS_LOGGING_H */