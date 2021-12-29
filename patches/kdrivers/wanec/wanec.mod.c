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
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x4d033975, "module_layout" },
	{ 0x3f234f3e, "wp_logger_input" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0x7ee91c1d, "_spin_trylock" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0xc12eb167, "wp_logger_level_hwec" },
	{ 0xb61f0f80, "del_timer" },
	{ 0xd0d8621b, "strlen" },
	{ 0x5113d3a4, "malloc_sizes" },
	{ 0x973873ab, "_spin_lock" },
	{ 0x4661e311, "__tracepoint_kmalloc" },
	{ 0x66351462, "device_destroy" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0xb1ca2692, "init_timer_key" },
	{ 0x999e8297, "vfree" },
	{ 0x3c2c5af5, "sprintf" },
	{ 0x7d11c268, "jiffies" },
	{ 0xe2d5255a, "strcmp" },
	{ 0x5487ec8, "wp_logger_level_default" },
	{ 0x77da8dee, "register_wanec_iface" },
	{ 0x8d3894f2, "_ctype" },
	{ 0x5152e605, "memcmp" },
	{ 0x2da418b5, "copy_to_user" },
	{ 0xc7fbd1f5, "device_create" },
	{ 0xeba165b4, "add_timer" },
	{ 0x6dcedb09, "kmem_cache_alloc" },
	{ 0x4df932b, "unregister_wanec_iface" },
	{ 0xdeeaedb, "wp_logger_repeating_message_filter" },
	{ 0x181631b1, "register_chrdev" },
	{ 0xf6ebc03b, "net_ratelimit" },
	{ 0x1d2e87c6, "do_gettimeofday" },
	{ 0x37a0cba, "kfree" },
	{ 0x2e60bace, "memcpy" },
	{ 0x9ef749e2, "unregister_chrdev" },
	{ 0xb4e817cd, "class_destroy" },
	{ 0x12aacf5c, "__class_create" },
	{ 0xf2a644fb, "copy_from_user" },
	{ 0xe914e41e, "strcpy" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=sdladrv,wanrouter";


MODULE_INFO(srcversion, "04C049BED8541540DFC0B81");
