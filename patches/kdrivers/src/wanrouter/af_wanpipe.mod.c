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
	{ 0x52c0a61d, "_write_unlock_irqrestore" },
	{ 0xff2ee19e, "sock_init_data" },
	{ 0x3d7c39ea, "_read_lock" },
	{ 0xe1b7029c, "print_tainted" },
	{ 0x19af2928, "sock_no_setsockopt" },
	{ 0x3aabb4fb, "sock_no_getsockopt" },
	{ 0xab978df6, "malloc_sizes" },
	{ 0x3adc4c7c, "remove_wait_queue" },
	{ 0x3093180f, "_write_lock_irqsave" },
	{ 0x60ea5fe7, "__tasklet_hi_schedule" },
	{ 0x8b61da01, "dev_get_by_name" },
	{ 0x63ecad53, "register_netdevice_notifier" },
	{ 0xae22d008, "sock_queue_rcv_skb" },
	{ 0xf2a90733, "skb_recv_datagram" },
	{ 0xea1d1e4b, "sock_rfree" },
	{ 0xfe769456, "unregister_netdevice_notifier" },
	{ 0xffd5a395, "default_wake_function" },
	{ 0x42d067ff, "skb_queue_purge" },
	{ 0x2fc7657d, "sock_no_socketpair" },
	{ 0x975c62af, "sk_alloc" },
	{ 0x1b7d4074, "printk" },
	{ 0x5730615d, "sk_free" },
	{ 0x5a2bc739, "dev_get_by_index" },
	{ 0xa5808bbf, "tasklet_init" },
	{ 0x149a799f, "dev_kfree_skb_any" },
	{ 0x6ab10cf1, "sock_no_shutdown" },
	{ 0x79ad224b, "tasklet_kill" },
	{ 0x1c53db6e, "skb_over_panic" },
	{ 0xb356ebea, "skb_queue_tail" },
	{ 0x8b4e4185, "inet_dgram_ops" },
	{ 0x19070091, "kmem_cache_alloc" },
	{ 0x9ceb163c, "memcpy_toiovec" },
	{ 0x9aebf873, "__alloc_skb" },
	{ 0x4f2fd580, "sock_register" },
	{ 0x4292364c, "schedule" },
	{ 0x89d282ea, "kfree_skb" },
	{ 0x6b2dc060, "dump_stack" },
	{ 0xf39bf4d9, "put_cmsg" },
	{ 0xf6ebc03b, "net_ratelimit" },
	{ 0x72270e35, "do_gettimeofday" },
	{ 0xa3d44f8c, "add_wait_queue" },
	{ 0x37a0cba, "kfree" },
	{ 0x2394a062, "sock_unregister" },
	{ 0x9fb3dd30, "memcpy_fromiovec" },
	{ 0x2e2ed056, "skb_dequeue" },
	{ 0x2b720703, "skb_free_datagram" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "939544ED75A6B7F66EE9888");
