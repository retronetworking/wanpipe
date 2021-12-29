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
	{ 0xc98aebe6, "wan_get_ip_address" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0xec7bc0d, "__mod_timer" },
	{ 0x6df6c4e0, "wan_run_wanrouter" },
	{ 0x4827a016, "del_timer" },
	{ 0x79aa04a2, "get_random_bytes" },
	{ 0xab978df6, "malloc_sizes" },
	{ 0xf6a5a6c8, "schedule_work" },
	{ 0x87cddf59, "_spin_lock_irqsave" },
	{ 0x1d26aa98, "sprintf" },
	{ 0x5ddb19f0, "in_dev_finish_destroy" },
	{ 0x7d11c268, "jiffies" },
	{ 0x651a840a, "wan_add_gateway" },
	{ 0x92cfbd9d, "netif_rx" },
	{ 0xfd74fadb, "wan_set_ip_address" },
	{ 0x5487ec8, "wp_logger_level_default" },
	{ 0x1b7d4074, "printk" },
	{ 0x2da418b5, "copy_to_user" },
	{ 0xa20fdde, "_spin_unlock_irqrestore" },
	{ 0x1c53db6e, "skb_over_panic" },
	{ 0x7dceceac, "capable" },
	{ 0x19070091, "kmem_cache_alloc" },
	{ 0x9aebf873, "__alloc_skb" },
	{ 0xdeeaedb, "wp_logger_repeating_message_filter" },
	{ 0x89d282ea, "kfree_skb" },
	{ 0xb08e0988, "skb_under_panic" },
	{ 0xd0b91f9b, "init_timer" },
	{ 0x37a0cba, "kfree" },
	{ 0x25da070, "snprintf" },
	{ 0x8235805b, "memmove" },
	{ 0x10818a0c, "dev_queue_xmit" },
	{ 0xf2a644fb, "copy_from_user" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=sdladrv,wanrouter";


MODULE_INFO(srcversion, "8FF014FB05CFEFEE63E5EE7");
