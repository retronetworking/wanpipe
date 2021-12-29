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
	{ 0x7da8156e, "__kmalloc" },
	{ 0xdcad5d16, "__mod_timer" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0x15413d5, "del_timer" },
	{ 0x1192e363, "malloc_sizes" },
	{ 0x14e57c7d, "class_simple_create" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0x2fd1d81c, "vfree" },
	{ 0x1d26aa98, "sprintf" },
	{ 0xda02d67, "jiffies" },
	{ 0xd7474566, "__copy_to_user_ll" },
	{ 0x1af40e18, "__copy_from_user_ll" },
	{ 0x1b7d4074, "printk" },
	{ 0x71c90087, "memcmp" },
	{ 0xe2f1e52c, "class_simple_device_remove" },
	{ 0x49e79940, "__cond_resched" },
	{ 0xc01b581d, "kmem_cache_alloc" },
	{ 0x2d30c0a, "register_chrdev" },
	{ 0xf1fa0f9d, "class_simple_destroy" },
	{ 0xf6ebc03b, "net_ratelimit" },
	{ 0x72270e35, "do_gettimeofday" },
	{ 0x37a0cba, "kfree" },
	{ 0x2e60bace, "memcpy" },
	{ 0xc192d491, "unregister_chrdev" },
	{ 0xc8abd94d, "class_simple_device_add" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=";

