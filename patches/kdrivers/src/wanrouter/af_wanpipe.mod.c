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
	{ 0xffc7ace6, "_write_unlock_irqrestore" },
	{ 0xe07434f4, "security_ops" },
	{ 0x81b6c38b, "sock_init_data" },
	{ 0xd890c09e, "__kfree_skb" },
	{ 0x5c69e2a7, "_read_lock" },
	{ 0x75e1db7c, "sock_no_setsockopt" },
	{ 0xe56b032c, "sock_no_getsockopt" },
	{ 0x1192e363, "malloc_sizes" },
	{ 0xc9cd3f05, "remove_wait_queue" },
	{ 0xb06e7659, "_write_lock_irqsave" },
	{ 0x60ea5fe7, "__tasklet_hi_schedule" },
	{ 0x3d124271, "dev_get_by_name" },
	{ 0xb9e5ab27, "_spin_lock" },
	{ 0x63ecad53, "register_netdevice_notifier" },
	{ 0x87a02e40, "skb_recv_datagram" },
	{ 0xd8c152cd, "raise_softirq_irqoff" },
	{ 0x1836387f, "sock_rfree" },
	{ 0xfe769456, "unregister_netdevice_notifier" },
	{ 0x2b3e91, "default_wake_function" },
	{ 0x3b5e3979, "bind_api_listen_to_protocol" },
	{ 0x56bc0cc2, "skb_queue_purge" },
	{ 0x94de6227, "sock_no_socketpair" },
	{ 0x3e568f89, "bind_api_to_protocol" },
	{ 0x3b604364, "unbind_api_listen_from_protocol" },
	{ 0xf393de3, "sk_alloc" },
	{ 0x1b7d4074, "printk" },
	{ 0xe59198f0, "alloc_skb" },
	{ 0xf75aebe3, "sk_free" },
	{ 0x9a4f9166, "dev_get_by_index" },
	{ 0xa5808bbf, "tasklet_init" },
	{ 0xec15a2f2, "_spin_unlock" },
	{ 0xf5fd2776, "sock_no_shutdown" },
	{ 0x79ad224b, "tasklet_kill" },
	{ 0xb382a803, "skb_over_panic" },
	{ 0xebddcd44, "skb_queue_tail" },
	{ 0xfc4a389d, "inet_dgram_ops" },
	{ 0xc01b581d, "kmem_cache_alloc" },
	{ 0x9ceb163c, "memcpy_toiovec" },
	{ 0x3cc85731, "sock_register" },
	{ 0x4292364c, "schedule" },
	{ 0x45ed1ce4, "register_wanpipe_api_socket" },
	{ 0x6b2dc060, "dump_stack" },
	{ 0xf39bf4d9, "put_cmsg" },
	{ 0x280f9f14, "__per_cpu_offset" },
	{ 0xd8565995, "_read_unlock" },
	{ 0xf6ebc03b, "net_ratelimit" },
	{ 0x72270e35, "do_gettimeofday" },
	{ 0xe861712c, "add_wait_queue" },
	{ 0xe69083b7, "sk_run_filter" },
	{ 0x37a0cba, "kfree" },
	{ 0xdfa7e4f4, "___pskb_trim" },
	{ 0x2394a062, "sock_unregister" },
	{ 0x9fb3dd30, "memcpy_fromiovec" },
	{ 0x42a02cdc, "skb_dequeue" },
	{ 0x387c78a5, "dev_ioctl" },
	{ 0x38eb6851, "unregister_wanpipe_api_socket" },
	{ 0xd82ae2d6, "skb_free_datagram" },
	{ 0xc19220e4, "per_cpu__softnet_data" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=wanrouter";

