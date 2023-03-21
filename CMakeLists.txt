cmake_minimum_required(VERSION 3.24)
project(riscv_pke C)

set(CMAKE_C_STANDARD 11)

include_directories(kernel)
include_directories(spike_interface)
include_directories(user)
include_directories(util)

add_executable(riscv_pke
        kernel/machine/minit.c
        kernel/machine/mtrap.c
        kernel/config.h
        kernel/elf.c
        kernel/elf.h
        kernel/hostfs.c
        kernel/hostfs.h
        kernel/kernel.c
        kernel/memlayout.h
        kernel/pmm.c
        kernel/pmm.h
        kernel/proc_file.c
        kernel/proc_file.h
        kernel/process.c
        kernel/process.h
        kernel/ramdev.c
        kernel/ramdev.h
        kernel/rfs.c
        kernel/rfs.h
        kernel/riscv.h
        kernel/sched.c
        kernel/sched.h
        kernel/strap.c
        kernel/strap.h
        kernel/syscall.c
        kernel/syscall.h
        kernel/vfs.c
        kernel/vfs.h
        kernel/vmm.c
        kernel/vmm.h
        spike_interface/atomic.h
        spike_interface/dts_parse.c
        spike_interface/dts_parse.h
        spike_interface/spike_file.c
        spike_interface/spike_file.h
        spike_interface/spike_htif.c
        spike_interface/spike_htif.h
        spike_interface/spike_memory.c
        spike_interface/spike_memory.h
        spike_interface/spike_utils.c
        spike_interface/spike_utils.h
        user/app_exec.c
        user/app_ls.c
        user/user_lib.c
        user/user_lib.h
        util/functions.h
        util/hash_table.c
        util/hash_table.h
        util/snprintf.c
        util/snprintf.h
        util/string.c
        util/string.h
        util/types.h)
