#include "kernel/riscv.h"
#include "kernel/process.h"
#include "spike_interface/spike_utils.h"
#include "kernel/elf.h"
#include "util/string.h"
static void handle_instruction_access_fault() { panic("Instruction access fault!"); }

static void handle_load_access_fault() { panic("Load access fault!"); }

static void handle_store_access_fault() { panic("Store/AMO access fault!"); }

static void handle_illegal_instruction() { panic("Illegal instruction!"); }

static void handle_misaligned_load() { panic("Misaligned Load!"); }

static void handle_misaligned_store() { panic("Misaligned AMO!"); }

// added @lab1_3
static void handle_timer() {
  int cpuid = 0;
  // setup the timer fired at next time (TIMER_INTERVAL from now)
  *(uint64*)CLINT_MTIMECMP(cpuid) = *(uint64*)CLINT_MTIMECMP(cpuid) + TIMER_INTERVAL;

  // setup a soft interrupt in sip (S-mode Interrupt Pending) to be handled in S-mode
  write_csr(sip, SIP_SSIP);
}
void print_errorline()
{
  sprint("print_errorline begin\n");
  /****************************************************************************/
  elf_ctx elfloader;
  elf_info info;
  info.f = spike_file_open("obj/app_errorline", O_RDONLY, 0);
  info.p = current;
  /*读取elf header到elfloader.ehdr中*/
  if(elf_init(&elfloader, &info)!=EL_OK)
    panic("elf init error\n");
  if (elf_load(&elfloader) != EL_OK) panic("Fail on loading elf.\n");

  uint64 shstrndx=elfloader.ehdr.shstrndx;//shstrndx的索引值
  uint64 shnum=elfloader.ehdr.shnum;//section header的数目
  elf_sect_header section_header[shnum];
  int i,off;
  for(i=0,off=elfloader.ehdr.shoff;i<shnum;i++,off+=sizeof(elf_sect_header))
  {
    /*读取 section header*/
    if(elf_fpread(&elfloader,(void*)(section_header+i),
                  sizeof(elf_sect_header),off)!=sizeof(elf_sect_header))
      panic("load section header into elfloader fail\n");
  }
  
  /*读取shstrtab*/
  char shstrtab[section_header[shstrndx].size];
  if(elf_fpread(&elfloader,(void*)&shstrtab,section_header[shstrndx].size,
              section_header[shstrndx].offset)!=section_header[shstrndx].size)
    panic("load section shstrtab fail");
  int idx_symtab=0,idx_strtab=0;
  for(int i=0;i<shnum;i++)
  {
    if(strcmp(shstrtab+section_header[i].name,".symtab")==0)
    {
      idx_symtab=i;
      break;
    }
  }
  for(int i=0;i<shnum;i++)
  {
    if(strcmp(shstrtab+section_header[i].name,".strtab")==0)
    {
      idx_strtab=i;
      break;
    }
  }  

  int idx_debug_line_section_header=0;
  for(int i=0;i<shnum;i++)
  {
    if(strcmp(shstrtab+section_header[i].name,".debug_line")==0)
    {
      idx_debug_line_section_header=i;
      break;
    }
  }
  /*读取debug_line*/
  uint64 debug_line_offset=section_header[idx_debug_line_section_header].offset;
  uint64 debug_line_size=section_header[idx_debug_line_section_header].size;
  char debug_line[debug_line_size];
  if(elf_fpread(&elfloader,(void*)debug_line,debug_line_size,
              debug_line_offset)!=debug_line_size)
    panic("load section debug_line fail");
  sprint("make_addr_line begin\n");
  make_addr_line(&elfloader, debug_line, debug_line_size);
  sprint("make_addr_line done\n");

  /*读取symtab*/
  int num_symbol=section_header[idx_symtab].size/sizeof(Elf64_Sym);
  Elf64_Sym symbol[num_symbol];
  if(elf_fpread(&elfloader,(void*)&symbol,section_header[idx_symtab].size,
              section_header[idx_symtab].offset)!=section_header[idx_symtab].size)
    panic("load section symtab fail");
  
  /*读取strtab*/
  char strtab[section_header[idx_strtab].size];
  if(elf_fpread(&elfloader,(void*)&strtab,section_header[idx_strtab].size,
              section_header[idx_strtab].offset)!=section_header[idx_strtab].size)
    panic("load section strtab fail");
  spike_file_close( info.f );  
/*****************************************************/
  sprint("print_errorline done\n");
}

void print_error_case(uint64 mcause)
{
  switch(mcause)
  {
    case CAUSE_FETCH_ACCESS:
      sprint("Instruction access fault!\n");
      break;
    case CAUSE_LOAD_ACCESS:
      sprint("Load access fault!\n");
    case CAUSE_STORE_ACCESS:
      sprint("Store/AMO access fault!\n");
      break;
    case CAUSE_ILLEGAL_INSTRUCTION:
      sprint("Illegal instruction!\n");
      break;
    case CAUSE_MISALIGNED_LOAD:
      sprint("Misaligned Load!\n");
      break;
    case CAUSE_MISALIGNED_STORE:
      sprint("Misaligned AMO!\n");
      break;
    default:
      sprint("machine trap(): unexpected mscause %p\n", mcause);
      sprint("            mepc=%p mtval=%p\n", read_csr(mepc), read_csr(mtval));
      sprint( "unexpected exception happened in M-mode.\n" );
      break;
  }
}
//
// handle_mtrap calls a handling function according to the type of a machine mode interrupt (trap).
//
void handle_mtrap() {
  uint64 mcause = read_csr(mcause);
  print_error_case(mcause);
  print_errorline();
  switch (mcause) {
    case CAUSE_MTIMER:
      handle_timer();
      break;
    case CAUSE_FETCH_ACCESS:
      handle_instruction_access_fault();
      break;
    case CAUSE_LOAD_ACCESS:
      handle_load_access_fault();
    case CAUSE_STORE_ACCESS:
      handle_store_access_fault();
      break;
    case CAUSE_ILLEGAL_INSTRUCTION:
      handle_illegal_instruction();

      break;
    case CAUSE_MISALIGNED_LOAD:
      handle_misaligned_load();
      break;
    case CAUSE_MISALIGNED_STORE:
      handle_misaligned_store();
      break;

    default:
      sprint("machine trap(): unexpected mscause %p\n", mcause);
      sprint("            mepc=%p mtval=%p\n", read_csr(mepc), read_csr(mtval));
      panic( "unexpected exception happened in M-mode.\n" );
      break;
  }
}
