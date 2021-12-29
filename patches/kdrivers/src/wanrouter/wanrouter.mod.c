#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

#undef unix
struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = __stringify(KBUILD_MODNAME),
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
};

static const struct modversion_info ____versions[]
__attribute_used__
__attribute__((section("__versions"))) = {
	{ 0x44d7b32c, "struct_module" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0x11dd8cd3, "single_open" },
	{ 0xa9b25754, "single_release" },
	{ 0x1192e363, "malloc_sizes" },
	{ 0x3d124271, "dev_get_by_name" },
	{ 0xb9e5ab27, "_spin_lock" },
	{ 0x5b4eb2e4, "seq_printf" },
	{ 0x9a3eea6f, "remove_proc_entry" },
	{ 0x1e90531b, "alloc_netdev" },
	{ 0x2fd1d81c, "vfree" },
	{ 0x94208e84, "ip_rt_ioctl" },
	{ 0x1d26aa98, "sprintf" },
	{ 0x4fb44432, "in_dev_finish_destroy" },
	{ 0x43edac59, "seq_read" },
	{ 0xd7474566, "__copy_to_user_ll" },
	{ 0x1af40e18, "__copy_from_user_ll" },
	{ 0x69baa705, "proc_mkdir" },
	{ 0x18cb22f, "proc_net" },
	{ 0x1b7d4074, "printk" },
	{ 0x71c90087, "memcmp" },
	{ 0x869c80ea, "devinet_ioctl" },
	{ 0xfbea4130, "free_netdev" },
	{ 0xe580ae37, "register_netdev" },
	{ 0x436006da, "call_usermodehelper" },
	{ 0xec15a2f2, "_spin_unlock" },
	{ 0x49e79940, "__cond_resched" },
	{ 0x7dceceac, "capable" },
	{ 0xc01b581d, "kmem_cache_alloc" },
	{ 0x2347aa4f, "skb_under_panic" },
	{ 0x9ac60d3a, "create_proc_entry" },
	{ 0x6e0326ff, "seq_lseek" },
	{ 0x37a0cba, "kfree" },
	{ 0x2e60bace, "memcpy" },
	{ 0xd80fb236, "unregister_netdev" },
	{ 0xa78a714d, "seq_release" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=";

