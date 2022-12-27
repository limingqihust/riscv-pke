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
  elf_ctx elfloader;
  elf_info info;
  info.f = spike_file_open("obj/app_print_backtrace", O_RDONLY, 0);
  // info.p = current;
  /*读取elf header到elfloader.ehdr中*/
  if(elf_init(&elfloader, &info)!=EL_OK)
    panic("elf init error\n");
  elf_sect_header sh_addr;
  int i;
  int off;/*当前读取的section header的地址*/
  for(i=0,off=elfloader.ehdr.shoff;i<elfloader.ehdr.shnum;i++,off+=sizeof(sh_addr))
  {
    /*读取 section header*/
    if(elf_fpread(&elfloader,(void*)&sh_addr,sizeof(sh_addr),off)!=sizeof(sh_addr))
      panic("load section header into elfloader fail\n");
    continue;
  }


  uint64 num_section_header=(uint64)elfloader.ehdr.shnum;//section header的数量
  uint64 address_the_first_section_header=(uint64)elfloader.ehdr.shoff;//第1个section header的地址
  uint64 size_section_header=(uint64)elfloader.ehdr.shentsize;//section header的大小
  uint64 index_string_table=(uint64)elfloader.ehdr.shstrndx;//string table的索引值



  //section header symtab的地址 symtab存储函数变量名
  uint64 section_header_symtab_address=address_the_first_section_header+15*size_section_header;

  //section header strtab的地址
  uint64 section_header_strtab_address=address_the_first_section_header+16*size_section_header;

  //section header shstrtab的地址
  uint64 section_header_shstrtab_address=address_the_first_section_header+17*size_section_header;

  elf_sect_header symtab_header;
  elf_sect_header strtab_header;
  elf_sect_header shstrtab_header;
  /*读取symtab*/
  if(elf_fpread(&elfloader,(void*)&symtab_header,sizeof(symtab_header),section_header_symtab_address)!=sizeof(symtab_header))
    panic("load section header fail");
  int num_symbol=symtab_header.sh_size/sizeof(Elf64_Sym);
  Elf64_Sym symbol[num_symbol];
  if(elf_fpread(&elfloader,(void*)&symbol,symtab_header.sh_size,symtab_header.sh_offset)!=symtab_header.sh_size)
    panic("load section symtab fail");
  
  
  /*读取strtab*/
  if(elf_fpread(&elfloader,(void*)&strtab_header,sizeof(strtab_header),section_header_strtab_address)!=sizeof(strtab_header))
    panic("load section header fail");
  char strtab[strtab_header.sh_size];
  if(elf_fpread(&elfloader,(void*)&strtab,strtab_header.sh_size,strtab_header.sh_offset)!=strtab_header.sh_size)
    panic("load section strtab fail");

  /*读取shstrtab*/
  if(elf_fpread(&elfloader,(void*)&shstrtab_header,sizeof(shstrtab_header),section_header_shstrtab_address)!=sizeof(shstrtab_header))
    panic("load section header fail");
  char shstrtab[shstrtab_header.sh_size];
  if(elf_fpread(&elfloader,(void*)&shstrtab,shstrtab_header.sh_size,shstrtab_header.sh_offset)!=shstrtab_header.sh_size)
    panic("load section shstrtab fail");
  spike_file_close( info.f );

  uint64 ra=tf->regs.ra;
  uint64 sp=tf->regs.sp;
  uint64 s0=tf->regs.s0;
  sp+=32;
  for(int i=0;i<backtrace_num;i++)
  {
    ra=*(uint64*)(sp+8);
    s0=*(uint64*)(sp);
    sp+=16;
    uint64 func_addr;
    if(i==0)
      func_addr=ra-0xe;
    else func_addr=ra-0xc;
    for(int j=0;j<num_symbol;j++)
    {
      if(symbol[j].st_value==func_addr && symbol[j].st_info==STT_FUNC)
      {
        sprint("%s\n",strtab+symbol[j].st_name);
        break;
      }
    }
  }
  
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
