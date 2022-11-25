/*
 * contains the implementation of all syscalls.
 */

#include <stdint.h>
#include <errno.h>

#include "util/types.h"
#include "syscall.h"
#include "string.h"
#include "process.h"
#include "util/functions.h"
#include "elf.h"
#include "spike_interface/spike_utils.h"

//
// implement the SYS_user_print syscall
//
ssize_t sys_user_print(const char* buf, size_t n) {
  sprint(buf);
  return 0;
}

//
// implement the SYS_user_exit syscall
//
ssize_t sys_user_exit(uint64 code) {
  sprint("User exit with code:%d.\n", code);
  // in lab1, PKE considers only one app (one process). 
  // therefore, shutdown the system when the app calls exit()
  shutdown(code);
}


ssize_t sys_user_print_backtrace(trapframe* tf,int backtrace_num) {
  //s0 该函数上一个函数的栈顶指针吗print_backtrace-f8-f7-f6
  sprint("sys_user_print_backtrace begin\n");
  uint64 ra=tf->regs.ra;
  uint64 sp=tf->regs.sp;
  uint64 s0=tf->regs.s0;
  sp+=32;
  for(int i=0;i<9;i++)
  {
    ra=*(uint64*)(sp+8);
    s0=*(uint64*)(sp);
    sp+=16;
    sprint("return address is %llx\n",ra);
  }

//读取elf
/*************************************************************************/
  arg_buf arg_bug_msg;

  // retrieve command line arguements
  size_t argc = parse_args(&arg_bug_msg);
  //elf loading. elf_ctx is defined in kernel/elf.h, used to track the loading process.
  elf_ctx elfloader;
  // elf_info is defined above, used to tie the elf file and its corresponding process.
  elf_info info;
  info.f = spike_file_open(arg_bug_msg.argv[0], O_RDONLY, 0);
  info.p = current;
  // IS_ERR_VALUE is a macro defined in spike_interface/spike_htif.h
  if (IS_ERR_VALUE(info.f)) panic("Fail on openning the input application program.\n");

  // init elfloader context. elf_init() is defined above.
  if (elf_init(&elfloader, &info) != EL_OK)
    panic("fail to init elfloader.\n");
  
  sprint("the shnum is %x\n",elfloader.ehdr.shnum);
  /**********************************************************************/
  sprint("sys_user_print_backtrace end\n");
  return 0;
}
//
// [a0]: the syscall number; [a1] ... [a7]: arguments to the syscalls.
// returns the code of success, (e.g., 0 means success, fail for otherwise)
//
long do_syscall(trapframe* tf,long a0, long a1, long a2, long a3, long a4, long a5, long a6, long a7) {
  switch (a0) {
    case SYS_user_print:
      return sys_user_print((const char*)a1, a2);
    case SYS_user_exit:
      return sys_user_exit(a1);
    case SYS_user_print_backtrace:
      //a1要打印的调用栈的层数 
      return sys_user_print_backtrace(tf,a1);
    default:
      panic("Unknown syscall %ld \n", a0);
  }
}
