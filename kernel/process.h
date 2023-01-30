#ifndef _PROC_H_
#define _PROC_H_

#include "riscv.h"

typedef struct trapframe_t {
  // space to store context (all common registers)
  /* offset:0   */ riscv_regs regs;

  // process's "user kernel" stack
  /* offset:248 */ uint64 kernel_sp;
  // pointer to smode_trap_handler
  /* offset:256 */ uint64 kernel_trap;
  // saved user process counter
  /* offset:264 */ uint64 epc;
}trapframe;

// code file struct, including directory index and file name char pointer
typedef struct {
    uint64 dir; 
    char *file;
} code_file;

// address-line number-file name table
typedef struct {
    uint64 addr, line, file;
} addr_line;

// the extremely simple definition of process, used for begining labs of PKE
typedef struct process_t {
  // pointing to the stack used in trap handling.
  uint64 kstack;
  // trapframe storing the context of a (User mode) process.
  trapframe* trapframe;

  // added @lab1_challenge2
  char *debugline; 
  char **dir;             // 所有代码文件的文件夹名称的字符串数组
  code_file *file;        // 所有代码文件的文件名字符串指针以及其文件夹路径在dir数组中的索引
  addr_line *line;        // 所有指令地址，代码行号，文件名在file数组中的索引三者的映射关系
  int line_ind;           //
}process;

void switch_to(process*);

extern process* current;

#endif
