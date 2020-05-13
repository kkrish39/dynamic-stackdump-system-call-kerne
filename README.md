# About the project
The aim of the project is to implement two system calls. One to enable a dump stack operation at the given address and 
another one for removing the added dump stack. 
- Triggering a dump can be achieved through dump_stack() function. 
- The insertion of dump stack at given address can be achieved through kprobes.

# About the implementation
- Kernel version - 3.19.8
- Tested board - Galelio Gen II

- I have defined two syscalls with the syscall numbers as follows:
- 359 - insdump(syscall_num, SYNBOL_NAME)
- 360 - rmdump(dump_id)

- I have modified kallsyms.c and exit.c to export is_kernel_text and to plug in "remove_dynamic_dump" a cleanup routine to remove all the process bounded by CONFIG_DYNAMIC_DUMP_STACK. 

- By default CONFIG_DYNAMIC_DUMP_STACK will not be enabled.

- For testing module symbols, I've used already created kernel modules.

# Motivation to plug in cleanup routine in exit.c
- With the thought process that, if we are instering a dump_stack for a long running process, it makes no difference between adding a kprobe and plugging in the cleanup routine in exit.c. 
- It may also reduce the overhead of if-condition check for inserting and removing the kprobe while insdump and rmdump.

# Steps to build and test the syscall
1)Go to the directory containing the old kernel.
2)Run the patch command as follows.
    patch -p1 < ../dynamic_dump_patch.patch
3)Enable CONFIG_DYNAMIC_DUMP_STACK configuration to make the syscalls to be operational.
    make menuconfig.
4)Include the cross-compilation tools in the PATH
    export PATH=path_to_sdk/sysroots/x86_64-pokysdk-linux/usr/bin/i586-poky-linux:$PATH
5)Cross-compile the kernel.
    ARCH=x86 LOCALVERSION= CROSS_COMPILE=i586-poky-linux- make -j4
6)Build and extract the kernel modules from the build to a target directory.
    ARCH=x86 LOCALVERSION= INSTALL_MOD_PATH=../galileo-install CROSS_COMPILE=i586-poky-linux- make modules_install
7)Extract the kernel image (bzImage) from the build to a target directory.
    cp arch/x86/boot/bzImage ../galileo-install/
8)Copy the new bzImage to SD card and boot the galileo board.

***Tested cases***
1) Inserting a dump_stack with owner process scope and making sure that child process is not triggering the dump_stack.
2) Trying to remove the dump_stack from the child process. (Failure case)
3) Trying to remove the dump_stack from the owner process who inserted it. (Positive case) 
4) Inserting a dump_stack with child process scope and making sure that child process is triggering the dump_stack.
5) Inserting a dump_stack with invalid symbol_name.
6) Inserting a dump_stack with all process scope and making sure that any process is triggering the dump_stack.
7) Making sure that all the inserted dumps are removed on process exit.


***Sample output (USER SPACE)***
Inside parent procees
Child Process created 
Dynamic dump created from child process(322) with id: 108 
Opened the file. 
About to removed the dynamic_dump from the thread function(323) with id: 108 
Dump removal status: -1 
About to remove the dynamic_dump from the created child process(322) with id: 108 
Dump removal status: 0 
Dynamic dump created from child process(322) with id: 109 
Error creating the dynamic dump from parent process(321) 
Dynamic dump created from parent process(321) with id: 110 
Opened the file. 
About to removed the dynamic_dump from the thread function(324) with id: 109 
Dump removal status: -1 

***Sample output (KERNEL SPACE)*** [Shortened]
[  277.284257] Successfully added a dump entry for process:316 with the dynamic dump_id: 105 
[  277.301681] Successfully added a dump entry for process:315 with the dynamic dump_id: 106 
[  277.313581] All processe access: pid: 316
[  277.317655] CPU: 0 PID: 316 Comm: syscall_tester Tainted: G           O   3.19.8-yocto-standard #54
[  277.320081] Hardware name: Intel Corp. QUARK/GalileoGen2, BIOS 0x01000200 01/01/2014
[  277.320081]  cd60984c cd60984c cd2c3f0c c1453931 cd2c3f1c c124cf05 c15afee5 0000013c
[  277.320081]  cd2c3f34 c10a27f2 cd609854 cd2c3f64 cd6097e0 c10378f1 cd2c3f4c c1027fa4
[  277.320081]  c10378f0 cd2c3f64 00000000 b7773ba8 cd2c3f5c c1002964 003d0f00 bff73e40
[  277.320081] Call Trace:
[  277.320081]  [<c1453931>] dump_stack+0x16/0x18
[  277.320081]  [<c124cf05>] pre_handler_dynamic_dump+0x65/0x90
[  277.320081]  [<c10a27f2>] aggr_pre_handler+0x32/0x70
[  277.320081]  [<c10378f1>] ? do_fork+0x1/0x320
[  277.320081]  [<c1027fa4>] kprobe_int3_handler+0xb4/0x130
[  277.320081]  [<c10378f0>] ? fork_idle+0xa0/0xa0
[  277.320081]  [<c1002964>] do_int3+0x44/0xa0
[  277.320081]  [<c1457e93>] int3+0x33/0x40
[  277.320081]  [<c110007b>] ? zap_page_range_single+0x2b/0xc0
[  277.320081]  [<c1100000>] ? unmap_single_vma+0x570/0x5c0
[  277.320081]  [<c10378f1>] ? do_fork+0x1/0x320
[  277.320081]  [<c1037c86>] ? SyS_clone+0x16/0x20
[  277.320081]  [<c1457344>] syscall_call+0x7/0x7
[  277.432052] Opening the rbprobe driver 
[  277.441932] Removing dump entry with id: 105 
[  277.540812] Successfully added a dump entry for process:316 with the dynamic dump_id: 107 
[  277.560963] All processe access: pid: 316
.
.
.
.
.
[  299.344956] Child or thread process access with parent: pid: 324
[  299.351032] CPU: 0 PID: 324 Comm: syscall_tester Tainted: G           O   3.19.8-yocto-standard #54
[  299.352796] Hardware name: Intel Corp. QUARK/GalileoGen2, BIOS 0x01000200 01/01/2014
[  299.352796]  cd435a8c cd435a8c cd769d70 c1453931 cd769d80 c124cee8 c15b03c4 00000144
[  299.352796]  cd769d98 c10a27f2 cd435a94 cd769dc8 cd435de0 d28c7021 cd769db0 c1027fa4
[  299.352796]  d28c7020 cd769dc8 00000000 cd7976e0 cd769dc0 c1002964 d28c9260 cd1f4ba8
[  299.352796] Call Trace:
[  299.352796]  [<c1453931>] dump_stack+0x16/0x18
[  299.352796]  [<c124cee8>] pre_handler_dynamic_dump+0x48/0x90
[  299.352796]  [<c10a27f2>] aggr_pre_handler+0x32/0x70
[  299.352796]  [<d28c7021>] ? rbprobe_driver_open+0x1/0x20 [dynamic_dump_stack]
[  299.352796]  [<c1027fa4>] kprobe_int3_handler+0xb4/0x130
[  299.352796]  [<d28c7020>] ? post_handler_dynamic_dump+0x10/0x10 [dynamic_dump_stack]
[  299.352796]  [<c1002964>] do_int3+0x44/0xa0
[  299.352796]  [<c1457e93>] int3+0x33/0x40
[  299.352796]  [<d28c7020>] ? post_handler_dynamic_dump+0x10/0x10 [dynamic_dump_stack]
[  299.352796]  [<d28c7021>] ? rbprobe_driver_open+0x1/0x20 [dynamic_dump_stack]
[  299.352796]  [<c111f2ac>] ? chrdev_open+0x7c/0x130
[  299.352796]  [<c11195ff>] do_dentry_open+0x18f/0x2c0
[  299.352796]  [<c111f230>] ? cdev_put+0x20/0x20
[  299.352796]  [<c111993c>] vfs_open+0x3c/0x50
[  299.352796]  [<c1128293>] do_last+0x1f3/0xf00
[  299.352796]  [<c1114800>] ? kmem_cache_alloc+0xe0/0x110
[  299.352796]  [<c1129009>] path_openat+0x69/0x530
[  299.352796]  [<c1058301>] ? set_next_entity+0xb1/0xe0
[  299.352796]  [<c1058ad6>] ? pick_next_task_fair+0xf6/0x170
[  299.352796]  [<c112a207>] do_filp_open+0x27/0x80
[  299.352796]  [<c1134121>] ? __alloc_fd+0x61/0xf0
[  299.352796]  [<c11295b0>] ? getname_flags+0xa0/0x150
[  299.352796]  [<c111ae3a>] do_sys_open+0x10a/0x210
[  299.352796]  [<c1454acd>] ? schedule+0x1d/0x60
[  299.352796]  [<c1039386>] ? do_exit+0x576/0x890
[  299.352796]  [<c111af58>] SyS_open+0x18/0x20
[  299.352796]  [<c1457344>] syscall_call+0x7/0x7
[  299.534142] Opening the rbprobe driver 
[  299.546380] Successfully added a dump entry for process:321 with the dynamic dump_id: 110 
[  299.596493] Cleanup Process: Removing the dump with id:109 and process id: 322
[  299.605974] Cleanup Process: Removing the dump with id:110 and process id: 321