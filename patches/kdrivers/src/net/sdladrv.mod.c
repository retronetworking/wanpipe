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
	{ 0xc51c186d, "kobject_put" },
	{ 0x70ecc9b2, "cdev_del" },
	{ 0x4ac7b024, "pci_bus_read_config_byte" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0x7e3f931f, "_spin_trylock" },
	{ 0xd2248aac, "cdev_init" },
	{ 0x2ca3da4e, "pci_release_region" },
	{ 0xd8e484f0, "register_chrdev_region" },
	{ 0xe1b7029c, "print_tainted" },
	{ 0xb722a9bd, "usb_init_urb" },
	{ 0xf26c4b72, "class_device_destroy" },
	{ 0xb5513e49, "class_device_create" },
	{ 0xab978df6, "malloc_sizes" },
	{ 0x1bcd461f, "_spin_lock" },
	{ 0xf6a5a6c8, "schedule_work" },
	{ 0x757a9fc, "usb_kill_urb" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0xcdb08c03, "pci_bus_write_config_word" },
	{ 0x87cddf59, "_spin_lock_irqsave" },
	{ 0x1d26aa98, "sprintf" },
	{ 0x3ce9cac3, "usb_unlink_urb" },
	{ 0x7d11c268, "jiffies" },
	{ 0xb2a606bf, "pci_set_master" },
	{ 0xc659d5a, "del_timer_sync" },
	{ 0x51ba71d3, "pci_set_dma_mask" },
	{ 0x46c0e86e, "usb_deregister" },
	{ 0x1b7d4074, "printk" },
	{ 0x5152e605, "memcmp" },
	{ 0xed5c73bf, "__tasklet_schedule" },
	{ 0x2da418b5, "copy_to_user" },
	{ 0xfe121239, "usb_control_msg" },
	{ 0xaec4759f, "vprintk" },
	{ 0x56134363, "pci_bus_write_config_dword" },
	{ 0x2e1de6c1, "class_create" },
	{ 0xa20fdde, "_spin_unlock_irqrestore" },
	{ 0xa5808bbf, "tasklet_init" },
	{ 0x9eac042a, "__ioremap" },
	{ 0x149a799f, "dev_kfree_skb_any" },
	{ 0x79ad224b, "tasklet_kill" },
	{ 0x5e22fdec, "cdev_add" },
	{ 0x1c53db6e, "skb_over_panic" },
	{ 0xb356ebea, "skb_queue_tail" },
	{ 0xbb67c73a, "usb_submit_urb" },
	{ 0x19070091, "kmem_cache_alloc" },
	{ 0x9aebf873, "__alloc_skb" },
	{ 0x78021fa2, "pci_bus_read_config_word" },
	{ 0x7561ed, "pci_bus_read_config_dword" },
	{ 0x4292364c, "schedule" },
	{ 0x6b2dc060, "dump_stack" },
	{ 0x19cacd0, "init_waitqueue_head" },
	{ 0xd0b91f9b, "init_timer" },
	{ 0x6989a769, "vsnprintf" },
	{ 0x59968f3c, "__wake_up" },
	{ 0xf6ebc03b, "net_ratelimit" },
	{ 0xb85ab97a, "kmem_cache_zalloc" },
	{ 0x72270e35, "do_gettimeofday" },
	{ 0xb5f46a8b, "pci_bus_write_config_byte" },
	{ 0x37a0cba, "kfree" },
	{ 0x801678, "flush_scheduled_work" },
	{ 0xedc03953, "iounmap" },
	{ 0x487fa848, "usb_register_driver" },
	{ 0x4870cf59, "class_destroy" },
	{ 0x65ddd69, "pci_get_device" },
	{ 0x2e2ed056, "skb_dequeue" },
	{ 0x25da070, "snprintf" },
	{ 0xdd994dbd, "pci_enable_device" },
	{ 0xf2a644fb, "copy_from_user" },
	{ 0x3a626247, "pci_request_region" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "3EB86DD88FB89484F38B4C5");
