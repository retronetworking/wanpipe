#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
};

static const struct modversion_info ____versions[]
__attribute_used__
__attribute__((section("__versions"))) = {
	{ 0x89e24b9c, "struct_module" },
	{ 0x3f234f3e, "wp_logger_input" },
	{ 0xc7a22fe1, "dev_change_flags" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0x16274ab6, "single_open" },
	{ 0x27ae86e5, "single_release" },
	{ 0xab978df6, "malloc_sizes" },
	{ 0xc7a4fbed, "rtnl_lock" },
	{ 0xade2d0d7, "sdla_get_hw_probe" },
	{ 0x8b61da01, "dev_get_by_name" },
	{ 0x1bcd461f, "_spin_lock" },
	{ 0x669e2d2f, "seq_printf" },
	{ 0x5d57df57, "remove_proc_entry" },
	{ 0xfb1d0a92, "alloc_netdev" },
	{ 0x2fd1d81c, "vfree" },
	{ 0x1d26aa98, "sprintf" },
	{ 0x5ddb19f0, "in_dev_finish_destroy" },
	{ 0xf98c26e5, "seq_read" },
	{ 0x5487ec8, "wp_logger_level_default" },
	{ 0xc288bfdc, "wanpipe_cdev_cfg_ctrl_create" },
	{ 0x6315b09, "proc_mkdir" },
	{ 0xe987619e, "proc_net" },
	{ 0x1b7d4074, "printk" },
	{ 0x5152e605, "memcmp" },
	{ 0x15e074de, "free_netdev" },
	{ 0x2da418b5, "copy_to_user" },
	{ 0x604efc6a, "register_netdev" },
	{ 0x6dd4c4d6, "wanpipe_cdev_free" },
	{ 0xbd4a533a, "sdla_get_hwinfo" },
	{ 0x7dceceac, "capable" },
	{ 0x19070091, "kmem_cache_alloc" },
	{ 0xdeeaedb, "wp_logger_repeating_message_filter" },
	{ 0x72183c, "call_usermodehelper_keys" },
	{ 0xdfd0f6c6, "sdla_get_hw_adptr_cnt" },
	{ 0xb08e0988, "skb_under_panic" },
	{ 0xd5028665, "create_proc_entry" },
	{ 0x40e4fec5, "wake_up_process" },
	{ 0xf6ebc03b, "net_ratelimit" },
	{ 0x8ea8a3d5, "seq_lseek" },
	{ 0x37a0cba, "kfree" },
	{ 0x828fe72a, "unregister_netdev" },
	{ 0xf4e1d60e, "seq_release" },
	{ 0xf2a644fb, "copy_from_user" },
	{ 0x6e720ff2, "rtnl_unlock" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=sdladrv";


MODULE_INFO(srcversion, "B6D0B4FBBAD94E571F4EF36");
