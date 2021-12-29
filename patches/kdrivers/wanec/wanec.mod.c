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
	{ 0x12da5bb2, "__kmalloc" },
	{ 0x7e3f931f, "_spin_trylock" },
	{ 0xec7bc0d, "__mod_timer" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0x4827a016, "del_timer" },
	{ 0xf26c4b72, "class_device_destroy" },
	{ 0xb5513e49, "class_device_create" },
	{ 0xab978df6, "malloc_sizes" },
	{ 0x1bcd461f, "_spin_lock" },
	{ 0x4e830a3e, "strnicmp" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0x2fd1d81c, "vfree" },
	{ 0x1d26aa98, "sprintf" },
	{ 0x7d11c268, "jiffies" },
	{ 0xf8c8ebb2, "register_wanec_iface" },
	{ 0x1b7d4074, "printk" },
	{ 0x5152e605, "memcmp" },
	{ 0x2da418b5, "copy_to_user" },
	{ 0x2e1de6c1, "class_create" },
	{ 0x19070091, "kmem_cache_alloc" },
	{ 0x4df932b, "unregister_wanec_iface" },
	{ 0x4086729e, "register_chrdev" },
	{ 0xd0b91f9b, "init_timer" },
	{ 0xf6ebc03b, "net_ratelimit" },
	{ 0x72270e35, "do_gettimeofday" },
	{ 0x37a0cba, "kfree" },
	{ 0x2e60bace, "memcpy" },
	{ 0xc192d491, "unregister_chrdev" },
	{ 0x4870cf59, "class_destroy" },
	{ 0xf2a644fb, "copy_from_user" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=wanrouter";


MODULE_INFO(srcversion, "64A1A96922CEFB0D956F7B4");
