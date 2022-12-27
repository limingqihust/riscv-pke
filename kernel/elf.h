#ifndef _ELF_H_
#define _ELF_H_

#include "util/types.h"
#include "process.h"
#include "spike_interface/spike_utils.h"
#define MAX_CMDLINE_ARGS 64
typedef union {
  uint64 buf[MAX_CMDLINE_ARGS];
  char *argv[MAX_CMDLINE_ARGS];
} arg_buf;
typedef struct elf_info_t {
  spike_file_t *f;
  process *p;
} elf_info;
// elf header structure
typedef struct elf_header_t {
  uint32 magic;
  uint8 elf[12];
  uint16 type;      /* Object file type */
  uint16 machine;   /* Architecture */
  uint32 version;   /* Object file version */
  uint64 entry;     /* Entry point virtual address */
  uint64 phoff;     /* Program header table file offset */
  uint64 shoff;     /* 第一个Section header的位置 Section header table file offset */
  uint32 flags;     /* Processor-specific flags */
  uint16 ehsize;    /* ELF header size in bytes */
  uint16 phentsize; /* Program header table entry size */
  uint16 phnum;     /* Program header table entry count */
  uint16 shentsize; /* Section header table entry size */
  uint16 shnum;     /* Section header table entry count */
  uint16 shstrndx;  /* Section header string table index */
} elf_header;

// Program segment header.
typedef struct elf_prog_header_t {
  uint32 type;   /* Segment type */
  uint32 flags;  /* Segment flags */
  uint64 off;    /* Segment file offset */
  uint64 vaddr;  /* Segment virtual address */
  uint64 paddr;  /* Segment physical address */
  uint64 filesz; /* Segment size in file */
  uint64 memsz;  /* Segment size in memory */
  uint64 align;  /* Segment alignment */
} elf_prog_header;

typedef struct elf_section_header_t
{
  uint32 sh_name;       /*section名称 为字符串在shstrtab中的偏移量*/
  uint32 sh_type;
  uint64 sh_flags;
  uint64 sh_addr;       /*section virtual address*/
  uint64 sh_offset;     /*section 在文件内的偏移*/
  uint64 sh_size;       /*该section的大小*/ 
  uint32 sh_link;
  uint32 sh_info;
  uint64 sh_addraligh;
  uint64 sh_entsize;
}elf_sect_header;


#define STT_NOTYPE 0
#define STT_SECTION 3
#define STT_FILE 4
#define STT_FUNC 18
typedef struct 
{
  uint32 st_name;
  uint8 st_info;
  uint8 st_other;
  uint16 st_shndx;
  uint64 st_value;
  uint64 st_size;
} Elf64_Sym;



#define ELF_MAGIC 0x464C457FU  // "\x7FELF" in little endian
#define ELF_PROG_LOAD 1

typedef enum elf_status_t {
  EL_OK = 0,

  EL_EIO,
  EL_ENOMEM,
  EL_NOTELF,
  EL_ERR,

} elf_status;

typedef struct elf_ctx_t {
  void *info;
  elf_header ehdr;
} elf_ctx;
size_t parse_args(arg_buf *arg_bug_msg);
elf_status elf_init(elf_ctx *ctx, void *info);
elf_status elf_load(elf_ctx *ctx);
uint64 elf_fpread(elf_ctx *ctx, void *dest, uint64 nb, uint64 offset);
void *elf_alloc_mb(elf_ctx *ctx, uint64 elf_pa, uint64 elf_va, uint64 size);

void load_bincode_from_host_elf(process *p);
#endif
