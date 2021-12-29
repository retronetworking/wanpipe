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
	{ 0x79aa04a2, "get_random_bytes" },
	{ 0xab978df6, "malloc_sizes" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0xfb1d0a92, "alloc_netdev" },
	{ 0x1d26aa98, "sprintf" },
	{ 0x7d11c268, "jiffies" },
	{ 0x92cfbd9d, "netif_rx" },
	{ 0x1b7d4074, "printk" },
	{ 0x15e074de, "free_netdev" },
	{ 0x2da418b5, "copy_to_user" },
	{ 0x604efc6a, "register_netdev" },
	{ 0x1902adf, "netpoll_trap" },
	{ 0x149a799f, "dev_kfree_skb_any" },
	{ 0x19070091, "kmem_cache_alloc" },
	{ 0xad97dcd, "ether_setup" },
	{ 0x37a0cba, "kfree" },
	{ 0x828fe72a, "unregister_netdev" },
	{ 0xf2a644fb, "copy_from_user" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "CC822D0DB350F96CD01C274");
