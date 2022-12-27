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
  /*读取elf header到elfloader.ehdr中*/
  if(elf_init(&elfloader, &info)!=EL_OK)
    panic("elf init error\n");
  elf_sect_header section_header[elfloader.ehdr.shnum];
  int i,off;
  for(i=0,off=elfloader.ehdr.shoff;i<elfloader.ehdr.shnum;i++,off+=sizeof(elf_sect_header))
  {
    /*读取 section header*/
    if(elf_fpread(&elfloader,(void*)(section_header+i),sizeof(elf_sect_header),off)!=sizeof(elf_sect_header))
      panic("load section header into elfloader fail\n");
  }

  uint64 num_section_header=(uint64)elfloader.ehdr.shnum;//section header的数量
  uint64 first_section_header_off=(uint64)elfloader.ehdr.shoff;//第1个section header的地址
  uint64 size_section_header=(uint64)elfloader.ehdr.shentsize;//section header的大小
  uint64 shstrndx=elfloader.ehdr.shstrndx;//shstrndx的索引值

  /*section header shstrtab的地址*/
  uint64 section_header_shstrtab_address=first_section_header_off+shstrndx*size_section_header;
  elf_sect_header shstrtab_header;

  /*读取shstrtab*/
  if(elf_fpread(&elfloader,(void*)&shstrtab_header,sizeof(shstrtab_header),section_header_shstrtab_address)!=sizeof(shstrtab_header))
    panic("load section header fail");
  char shstrtab[shstrtab_header.sh_size];
  if(elf_fpread(&elfloader,(void*)&shstrtab,shstrtab_header.sh_size,shstrtab_header.sh_offset)!=shstrtab_header.sh_size)
    panic("load section shstrtab fail");
  int idx_symtab=0,idx_strtab=0;
  for(int i=0;i<elfloader.ehdr.shnum;i++)
  {
    if(strcmp(shstrtab+section_header[i].sh_name,".symtab")==0)
    {
      idx_symtab=i;
      break;
    }
  }
  for(int i=0;i<elfloader.ehdr.shnum;i++)
  {
    if(strcmp(shstrtab+section_header[i].sh_name,".strtab")==0)
    {
      idx_strtab=i;
      break;
    }
  }  
  /*读取symtab*/
  int num_symbol=section_header[idx_symtab].sh_size/sizeof(Elf64_Sym);
  Elf64_Sym symbol[num_symbol];
  if(elf_fpread(&elfloader,(void*)&symbol,section_header[idx_symtab].sh_size,section_header[idx_symtab].sh_offset)!=section_header[idx_symtab].sh_size)
    panic("load section symtab fail");


  Elf64_Sym temp;
  for (int i=0;i<num_symbol;i++) 
  {
    for(int j=0;j<num_symbol-i-1;j++) 
    {
      if(symbol[j].st_value>symbol[j+1].st_value)
      {
        temp=symbol[j];
        symbol[j]=symbol[j+1];
        symbol[j+1]=temp;
      }
    }
  }

  /*读取strtab*/
  char strtab[section_header[idx_strtab].sh_size];
  if(elf_fpread(&elfloader,(void*)&strtab,section_header[idx_strtab].sh_size,section_header[idx_strtab].sh_offset)!=section_header[idx_strtab].sh_size)
    panic("load section strtab fail");
  spike_file_close( info.f );

  uint64 ra=tf->regs.ra;
  uint64 sp=tf->regs.sp;
  uint64 s0=tf->regs.s0;
  uint64 nxt_sp;
  s0=*(uint64*)(sp+24);
  sp+=32;
  nxt_sp=s0;
  ra=*(uint64*)(sp+8);//函数的返回地址
  s0=*(uint64*)(sp);//函数保存的栈顶指针
  // sp+=16;
  sp=nxt_sp;
  /*print_backtrace->f8*/
  for(int i=0;i<backtrace_num;i++)
  {     
    if(ra==0)
      break;
    for(int j=0;j<num_symbol;j++)
    {
      if(symbol[num_symbol-j-1].st_value<ra)
      {
        sprint("%s\n",strtab+symbol[num_symbol-j-1].st_name);
        break;
      }
    }
    nxt_sp=s0;
    ra=*(uint64*)(sp+8);
    s0=*(uint64*)(sp);
    // sp+=16;
    sp=nxt_sp;
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
