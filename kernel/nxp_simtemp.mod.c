#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xc5e973a2, "misc_deregister" },
	{ 0xf8633f5c, "platform_driver_unregister" },
	{ 0x535f4f5f, "hrtimer_init" },
	{ 0x16ab4215, "__wake_up" },
	{ 0xd272d446, "__fentry__" },
	{ 0x5a844b26, "__x86_indirect_thunk_rax" },
	{ 0xe8213e80, "_printk" },
	{ 0xd272d446, "__stack_chk_fail" },
	{ 0xe90be339, "platform_device_register" },
	{ 0xd09b06f5, "kstrtoint" },
	{ 0x9ae2aef7, "platform_device_unregister" },
	{ 0xed75f9ac, "sysfs_create_group" },
	{ 0x5fa07cc0, "hrtimer_start_range_ns" },
	{ 0xbf0e63c7, "misc_register" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0x092a35a2, "_copy_to_user" },
	{ 0x8a2730a2, "__platform_driver_register" },
	{ 0xa6c58518, "sysfs_remove_group" },
	{ 0xdd6830c7, "sprintf" },
	{ 0x97acb853, "ktime_get" },
	{ 0xc01aafd2, "get_random_u32" },
	{ 0x36a36ab1, "hrtimer_cancel" },
	{ 0x49fc4616, "hrtimer_forward" },
	{ 0x70eca2ca, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0xc5e973a2,
	0xf8633f5c,
	0x535f4f5f,
	0x16ab4215,
	0xd272d446,
	0x5a844b26,
	0xe8213e80,
	0xd272d446,
	0xe90be339,
	0xd09b06f5,
	0x9ae2aef7,
	0xed75f9ac,
	0x5fa07cc0,
	0xbf0e63c7,
	0xd272d446,
	0x092a35a2,
	0x8a2730a2,
	0xa6c58518,
	0xdd6830c7,
	0x97acb853,
	0xc01aafd2,
	0x36a36ab1,
	0x49fc4616,
	0x70eca2ca,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"misc_deregister\0"
	"platform_driver_unregister\0"
	"hrtimer_init\0"
	"__wake_up\0"
	"__fentry__\0"
	"__x86_indirect_thunk_rax\0"
	"_printk\0"
	"__stack_chk_fail\0"
	"platform_device_register\0"
	"kstrtoint\0"
	"platform_device_unregister\0"
	"sysfs_create_group\0"
	"hrtimer_start_range_ns\0"
	"misc_register\0"
	"__x86_return_thunk\0"
	"_copy_to_user\0"
	"__platform_driver_register\0"
	"sysfs_remove_group\0"
	"sprintf\0"
	"ktime_get\0"
	"get_random_u32\0"
	"hrtimer_cancel\0"
	"hrtimer_forward\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "413D9F1EB969D879988CFEE");
