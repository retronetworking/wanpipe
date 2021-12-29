#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,6) || defined(WANPIPE_MOD_266_FORCE_UPDATE)
#undef unix
struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = __stringify(KBUILD_MODNAME),
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
};
#endif

static const struct modversion_info ____versions[]
__attribute__((section("__versions"))) = {

};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=wanrouter,sdladrv";

