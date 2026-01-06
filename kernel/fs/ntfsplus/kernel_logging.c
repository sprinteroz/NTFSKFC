/*
 *
 * Author: Darryl Bennett, owner of MagDriveX
 * Created: January 6, 2026
 * NTFSPLUS Kernel Module - Logging System
 * Converted from ntfs-3g logging.c to kernel space
 * GPL Compliant - Kernel Space Adaptation
 */

#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/module.h>
#include "kernel_types.h"

/* Logging levels - kernel compatible */
#define NTFSPLUS_LOG_LEVEL_ERROR     0
#define NTFSPLUS_LOG_LEVEL_WARNING   1
#define NTFSPLUS_LOG_LEVEL_INFO      2
#define NTFSPLUS_LOG_LEVEL_DEBUG     3
#define NTFSPLUS_LOG_LEVEL_TRACE     4

/* Current logging level - can be set via module parameter */
static int ntfsplus_log_level = NTFSPLUS_LOG_LEVEL_INFO;

/* Module parameter for log level */
module_param(ntfsplus_log_level, int, 0644);
MODULE_PARM_DESC(ntfsplus_log_level, "NTFSPLUS logging level (0=error,1=warning,2=info,3=debug,4=trace)");

/*
 * ntfsplus_log_error - Log error messages
 * Kernel equivalent of ntfs_log_error
 */
void ntfsplus_log_error(const char *format, ...)
{
    va_list args;
    char buf[256];

    if (ntfsplus_log_level >= NTFSPLUS_LOG_LEVEL_ERROR) {
        va_start(args, format);
        vsnprintf(buf, sizeof(buf), format, args);
        va_end(args);
        pr_err("NTFSPLUS ERROR: %s\n", buf);
    }
}

/*
 * ntfsplus_log_warning - Log warning messages
 * Kernel equivalent of ntfs_log_warning
 */
void ntfsplus_log_warning(const char *format, ...)
{
    va_list args;
    char buf[256];

    if (ntfsplus_log_level >= NTFSPLUS_LOG_LEVEL_WARNING) {
        va_start(args, format);
        vsnprintf(buf, sizeof(buf), format, args);
        va_end(args);
        pr_warn("NTFSPLUS WARNING: %s\n", buf);
    }
}

/*
 * ntfsplus_log_info - Log info messages
 * Kernel equivalent of ntfs_log_info
 */
void ntfsplus_log_info(const char *format, ...)
{
    va_list args;
    char buf[256];

    if (ntfsplus_log_level >= NTFSPLUS_LOG_LEVEL_INFO) {
        va_start(args, format);
        vsnprintf(buf, sizeof(buf), format, args);
        va_end(args);
        pr_info("NTFSPLUS INFO: %s\n", buf);
    }
}

/*
 * ntfsplus_log_debug - Log debug messages
 * Kernel equivalent of ntfs_log_debug
 */
void ntfsplus_log_debug(const char *format, ...)
{
    va_list args;
    char buf[256];

    if (ntfsplus_log_level >= NTFSPLUS_LOG_LEVEL_DEBUG) {
        va_start(args, format);
        vsnprintf(buf, sizeof(buf), format, args);
        va_end(args);
        pr_debug("NTFSPLUS DEBUG: %s\n", buf);
    }
}

/*
 * ntfsplus_log_trace - Log trace messages
 * Kernel equivalent of ntfs_log_trace
 */
void ntfsplus_log_trace(const char *format, ...)
{
    va_list args;
    char buf[256];

    if (ntfsplus_log_level >= NTFSPLUS_LOG_LEVEL_TRACE) {
        va_start(args, format);
        vsnprintf(buf, sizeof(buf), format, args);
        va_end(args);
        pr_debug("NTFSPLUS TRACE: %s\n", buf);
    }
}

/*
 * ntfsplus_log_enter - Log function entry
 * Kernel equivalent of ntfs_log_enter
 */
void ntfsplus_log_enter(const char *format, ...)
{
    va_list args;
    char buf[256];

    if (ntfsplus_log_level >= NTFSPLUS_LOG_LEVEL_TRACE) {
        va_start(args, format);
        vsnprintf(buf, sizeof(buf), format, args);
        va_end(args);
        pr_debug("NTFSPLUS ENTER: %s\n", buf);
    }
}

/*
 * ntfsplus_log_leave - Log function exit
 * Kernel equivalent of ntfs_log_leave
 */
void ntfsplus_log_leave(const char *format, ...)
{
    va_list args;
    char buf[256];

    if (ntfsplus_log_level >= NTFSPLUS_LOG_LEVEL_TRACE) {
        va_start(args, format);
        vsnprintf(buf, sizeof(buf), format, args);
        va_end(args);
        pr_debug("NTFSPLUS LEAVE: %s\n", buf);
    }
}

/*
 * ntfsplus_log_perror - Log error with perror-style message
 * Kernel equivalent of ntfs_log_perror
 */
void ntfsplus_log_perror(const char *format, ...)
{
    va_list args;
    char msg[256];

    if (ntfsplus_log_level >= NTFSPLUS_LOG_LEVEL_ERROR) {
        va_start(args, format);
        vsnprintf(msg, sizeof(msg), format, args);
        va_end(args);

        pr_err("NTFSPLUS ERROR: %s\n", msg);
    }
}

/*
 * ntfsplus_log_verbose - Log verbose messages
 * Kernel equivalent of ntfs_log_verbose
 */
void ntfsplus_log_verbose(const char *format, ...)
{
    va_list args;
    char buf[256];

    if (ntfsplus_log_level >= NTFSPLUS_LOG_LEVEL_INFO) {
        va_start(args, format);
        vsnprintf(buf, sizeof(buf), format, args);
        va_end(args);
        pr_info("NTFSPLUS VERBOSE: %s\n", buf);
    }
}

/*
 * ntfsplus_set_log_level - Set the current logging level
 */
void ntfsplus_set_log_level(int level)
{
    if (level >= NTFSPLUS_LOG_LEVEL_ERROR && level <= NTFSPLUS_LOG_LEVEL_TRACE) {
        ntfsplus_log_level = level;
        pr_info("NTFSPLUS: Log level set to %d\n", level);
    } else {
        pr_warn("NTFSPLUS: Invalid log level %d, keeping %d\n", level, ntfsplus_log_level);
    }
}

/*
 * ntfsplus_get_log_level - Get the current logging level
 */
int ntfsplus_get_log_level(void)
{
    return ntfsplus_log_level;
}

/*
 * Compatibility macros for existing ntfs-3g code
 * These allow gradual conversion of logging calls
 */
#define ntfs_log_error ntfsplus_log_error
#define ntfs_log_warning ntfsplus_log_warning
#define ntfs_log_info ntfsplus_log_info
#define ntfs_log_debug ntfsplus_log_debug
#define ntfs_log_trace ntfsplus_log_trace
#define ntfs_log_enter ntfsplus_log_enter
#define ntfs_log_leave ntfsplus_log_leave
#define ntfs_log_perror ntfsplus_log_perror
#define ntfs_log_verbose ntfsplus_log_verbose