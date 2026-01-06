#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xa61fd7aa, "__check_object_size" },
	{ 0x5657e2ef, "d_instantiate" },
	{ 0x558c294b, "_copy_to_iter" },
	{ 0xdf4bee3d, "alloc_workqueue" },
	{ 0xd710adbf, "__kmalloc_noprof" },
	{ 0xa53f4e29, "memmove" },
	{ 0x81cc768b, "new_inode" },
	{ 0x49733ad6, "queue_work_on" },
	{ 0xb03fd70b, "unregister_filesystem" },
	{ 0x692d8520, "d_make_root" },
	{ 0x39222a6d, "sb_set_blocksize" },
	{ 0xa53f4e29, "memcpy" },
	{ 0xcb8b6ec6, "kfree" },
	{ 0xd36b6cbe, "iput" },
	{ 0x37197a78, "vsnprintf" },
	{ 0xb03fd70b, "register_filesystem" },
	{ 0xf5d8d228, "rb_insert_color" },
	{ 0xde338d9a, "_raw_spin_lock" },
	{ 0xd272d446, "__fentry__" },
	{ 0x1c434a31, "simple_inode_init_ts" },
	{ 0x5a844b26, "__x86_indirect_thunk_rax" },
	{ 0x9174f503, "kill_block_super" },
	{ 0xe8213e80, "_printk" },
	{ 0xd272d446, "__stack_chk_fail" },
	{ 0xd710adbf, "__kmalloc_large_noprof" },
	{ 0x9479a1e8, "strnlen" },
	{ 0x5a844b26, "__x86_indirect_thunk_rdx" },
	{ 0x90a48d82, "__ubsan_handle_out_of_bounds" },
	{ 0x4cddec69, "simple_getattr" },
	{ 0x1dd0ae19, "__brelse" },
	{ 0xbd03ed67, "random_kmalloc_seed" },
	{ 0xbeb1d261, "destroy_workqueue" },
	{ 0xf5d8d228, "rb_erase" },
	{ 0x680628e7, "ktime_get_real_ts64" },
	{ 0xc1e6c71e, "__mutex_init" },
	{ 0x885eccdf, "__bread_gfp" },
	{ 0xe54e0a6b, "__fortify_panic" },
	{ 0x27683a56, "memset" },
	{ 0x5a844b26, "__x86_indirect_thunk_r10" },
	{ 0xbeb1d261, "__flush_workqueue" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0xe9c7272b, "__kmem_cache_create_args" },
	{ 0x888b8f57, "strcmp" },
	{ 0x058c185a, "jiffies" },
	{ 0x82fd7238, "__ubsan_handle_shift_out_of_bounds" },
	{ 0x4664303d, "generic_read_dir" },
	{ 0x5657e2ef, "d_add" },
	{ 0xb2fda715, "mount_bdev" },
	{ 0x23f25c0a, "__dynamic_pr_debug" },
	{ 0x70db3fe4, "__kmalloc_cache_noprof" },
	{ 0x28d98db7, "_copy_from_iter" },
	{ 0x43a349ca, "strlen" },
	{ 0xc2614bbe, "param_ops_int" },
	{ 0x6907ad13, "__mark_inode_dirty" },
	{ 0xde338d9a, "_raw_spin_unlock" },
	{ 0x81d6dd8f, "generic_file_llseek" },
	{ 0xfed1e3bc, "kmalloc_caches" },
	{ 0xfbe26b10, "krealloc_noprof" },
	{ 0x4f721e4e, "kmem_cache_destroy" },
	{ 0xba157484, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0xa61fd7aa,
	0x5657e2ef,
	0x558c294b,
	0xdf4bee3d,
	0xd710adbf,
	0xa53f4e29,
	0x81cc768b,
	0x49733ad6,
	0xb03fd70b,
	0x692d8520,
	0x39222a6d,
	0xa53f4e29,
	0xcb8b6ec6,
	0xd36b6cbe,
	0x37197a78,
	0xb03fd70b,
	0xf5d8d228,
	0xde338d9a,
	0xd272d446,
	0x1c434a31,
	0x5a844b26,
	0x9174f503,
	0xe8213e80,
	0xd272d446,
	0xd710adbf,
	0x9479a1e8,
	0x5a844b26,
	0x90a48d82,
	0x4cddec69,
	0x1dd0ae19,
	0xbd03ed67,
	0xbeb1d261,
	0xf5d8d228,
	0x680628e7,
	0xc1e6c71e,
	0x885eccdf,
	0xe54e0a6b,
	0x27683a56,
	0x5a844b26,
	0xbeb1d261,
	0xd272d446,
	0xe9c7272b,
	0x888b8f57,
	0x058c185a,
	0x82fd7238,
	0x4664303d,
	0x5657e2ef,
	0xb2fda715,
	0x23f25c0a,
	0x70db3fe4,
	0x28d98db7,
	0x43a349ca,
	0xc2614bbe,
	0x6907ad13,
	0xde338d9a,
	0x81d6dd8f,
	0xfed1e3bc,
	0xfbe26b10,
	0x4f721e4e,
	0xba157484,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"__check_object_size\0"
	"d_instantiate\0"
	"_copy_to_iter\0"
	"alloc_workqueue\0"
	"__kmalloc_noprof\0"
	"memmove\0"
	"new_inode\0"
	"queue_work_on\0"
	"unregister_filesystem\0"
	"d_make_root\0"
	"sb_set_blocksize\0"
	"memcpy\0"
	"kfree\0"
	"iput\0"
	"vsnprintf\0"
	"register_filesystem\0"
	"rb_insert_color\0"
	"_raw_spin_lock\0"
	"__fentry__\0"
	"simple_inode_init_ts\0"
	"__x86_indirect_thunk_rax\0"
	"kill_block_super\0"
	"_printk\0"
	"__stack_chk_fail\0"
	"__kmalloc_large_noprof\0"
	"strnlen\0"
	"__x86_indirect_thunk_rdx\0"
	"__ubsan_handle_out_of_bounds\0"
	"simple_getattr\0"
	"__brelse\0"
	"random_kmalloc_seed\0"
	"destroy_workqueue\0"
	"rb_erase\0"
	"ktime_get_real_ts64\0"
	"__mutex_init\0"
	"__bread_gfp\0"
	"__fortify_panic\0"
	"memset\0"
	"__x86_indirect_thunk_r10\0"
	"__flush_workqueue\0"
	"__x86_return_thunk\0"
	"__kmem_cache_create_args\0"
	"strcmp\0"
	"jiffies\0"
	"__ubsan_handle_shift_out_of_bounds\0"
	"generic_read_dir\0"
	"d_add\0"
	"mount_bdev\0"
	"__dynamic_pr_debug\0"
	"__kmalloc_cache_noprof\0"
	"_copy_from_iter\0"
	"strlen\0"
	"param_ops_int\0"
	"__mark_inode_dirty\0"
	"_raw_spin_unlock\0"
	"generic_file_llseek\0"
	"kmalloc_caches\0"
	"krealloc_noprof\0"
	"kmem_cache_destroy\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "6953375B971FA2BE195E134");
