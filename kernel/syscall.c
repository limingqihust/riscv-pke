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
  sprint("**************************************************\n");
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

  
  /*读取elf************************************************************************/
  elf_ctx elfloader;
  elf_info info;
  info.f = spike_file_open("./obj/app_print_backtrace", O_RDONLY, 0);
  info.p = current;
  /*读取elf header到elfloader.ehdr中*/
  if(elf_init(&elfloader, &info)!=EL_OK)
    sprint("elf init error\n");
  sprint("elf init done\n");
  sprint("shoff is %llx\n",(uint64)elfloader.ehdr.shoff);
  sprint("shentsize is %llx\n",(uint64)elfloader.ehdr.shentsize);
  sprint("shnum is %llx\n",(uint64)elfloader.ehdr.shnum);
  sprint("shstrndx is %llx\n",(uint64)elfloader.ehdr.shstrndx);
  sprint("load elf header into elfloader.ehdr done\n");
  /*section header size is 64 byte*/
  /*section header offset is 0x39d8(14808)*/
  /*section header num is 17*/

// #define load_prog
#define load_section

#ifdef load_prog
  /*将section header和section读取到elfloader*/
  // elf_prog_header structure is defined in kernel/elf.h
  elf_prog_header ph_addr;
  int i, off;
  // traverse the elf program segment headers
  for (i = 0, off = elfloader.ehdr.phoff; i < elfloader.ehdr.phnum; i++, off += sizeof(ph_addr)) 
  {
    // read segment headers
    if (elf_fpread(&elfloader, (void *)&ph_addr, sizeof(ph_addr), off) != sizeof(ph_addr)) sprint("load section header fail\n");

    if (ph_addr.type != ELF_PROG_LOAD)
      continue;
    if (ph_addr.memsz < ph_addr.filesz) sprint("load section header fail\n");
    if (ph_addr.vaddr + ph_addr.memsz < ph_addr.vaddr) sprint("load section header fail\n");
    sprint("load section header done\n");
    sprint("off is %llx\n",ph_addr.off);
    sprint("vaddr is %llx\n",ph_addr.vaddr);
    sprint("paddr is %llx\n",ph_addr.paddr);
    sprint("filesz is %llx\n",ph_addr.filesz);
    sprint("memsz is %llx\n",ph_addr.memsz);
    // allocate memory block before elf loading
    void *dest = elf_alloc_mb(&elfloader, ph_addr.vaddr, ph_addr.vaddr, ph_addr.memsz);

    // actual loading
    if (elf_fpread(&elfloader, dest, ph_addr.memsz, ph_addr.off) != ph_addr.memsz)
      sprint("load section fail\n");
  }
#endif



#ifdef load_section
  elf_section_header sh_addr;
  int i;
  int off;/*当前读取的section header的地址*/
  for(i=0,off=elfloader.ehdr.shoff;i<elfloader.ehdr.shnum;i++,off+=sizeof(sh_addr))
  {
    sprint("-----------------------------------\n");
    /*load section header*/
    if(elf_fpread(&elfloader,(void*)&sh_addr,sizeof(sh_addr),off)!=sizeof(sh_addr))
    {
      sprint("load section header into elfloader fail\n");
      break;
    }
    sprint("load section header %d done\n",i);
    sprint("information of the section header\n");
    sprint("the sh_name is %x\n",sh_addr.sh_name);
    sprint("the sh_type is %llx\n",sh_addr.sh_type);
    sprint("the sh_addr is %llx\n",sh_addr.sh_addr);
    sprint("the sh_offset is %llx\n",sh_addr.sh_offset);
    sprint("the sh_size is %llx\n",sh_addr.sh_size);
    // if(sh_addr.sh_type!=2&&sh_addr.sh_type!=3)
    // {
    //   sprint("not section\n");
    //   continue;
    // }
    continue;
    /*为load section分配memory*/
    void* dest=elf_alloc_mb(&elfloader,sh_addr.sh_addr,sh_addr.sh_addr,sh_addr.sh_size);
    sprint("alloc memory for section done\n");
    if (elf_fpread(&elfloader, dest, sh_addr.sh_size, sh_addr.sh_offset) != sh_addr.sh_size)
    {
      sprint("load section fail");
      break;
    }
    sprint("load setion %d done\n",i);
    sprint("-----------------------------------\n");
  }

  sprint("load section done\n");
#endif
  /*读取elf完毕************************************************************************/
  
  uint64 num_section_header=(uint64)elfloader.ehdr.shnum;//section header的数量
  uint64 address_the_first_section_header=(uint64)elfloader.ehdr.shoff;//第1个section header的地址
  uint64 size_section_header=(uint64)elfloader.ehdr.shentsize;//section header的大小
  uint64 index_string_table=(uint64)elfloader.ehdr.shstrndx;//string table的索引值

  
  //the start address of the sections headers is 0x39d8
  //the size of one section header is 64 bytes
  


  //section header symtab的地址 symtab存储函数变量名
  uint64 section_header_symtab_address=address_the_first_section_header+14*size_section_header;

  //section header strtab的地址
  uint64 section_header_strtab_address=address_the_first_section_header+15*size_section_header;

  //section header shstrtab的地址
  uint64 section_header_shstrtab_address=address_the_first_section_header+16*size_section_header;

  //the offset of the section header symtab is 0x3d58
  //the offset of the section header strtab is 0x3d98
  //the offset of the section header shstrtab is 0x3dd8
  sprint("%llx %llx %llx\n",section_header_symtab_address,section_header_strtab_address,section_header_shstrtab_address);

  // section header symtab中name的值 即字符串"symtab"在shstrtab中的偏移量 4 byte
  uint32 symtab_name=*(uint32*)(section_header_symtab_address);

  // void* content_section_header_symtab;
  // elf_fpread(&elfloader,content_section_header_symtab,64,section_header_symtab_address);
  // sprint("the offset of section header symtab name is %x",*(uint32*)content_section_header_symtab);

  //section shstrtab的地址
  uint64 shstrtab_offset=*(uint64*)(section_header_shstrtab_address+24);
  // section header symtab中的offset 即section symtab在文件中的偏移量

  uint64 symtab_offset=*(uint64*)(section_header_symtab_address+24);
  
  
  
  //the offset of the symtab section is 0x3588
  
  
  spike_file_close( info.f );
  /**********************************************************************/
  sprint("**************************************************\n");
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
