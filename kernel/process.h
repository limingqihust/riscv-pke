#ifndef _PROC_H_
#define _PROC_H_

#include "riscv.h"


#define MB_FREE 0
#define MB_MALLOCED 1
#define VMA_HEAP 1
typedef struct trapframe_t {
  // space to store context (all common registers)
  /* offset:0   */ riscv_regs regs;

  // process's "user kernel" stack
  /* offset:248 */ uint64 kernel_sp;
  // pointer to smode_trap_handler
  /* offset:256 */ uint64 kernel_trap;
  // saved user process counter
  /* offset:264 */ uint64 epc;

  // kernel page table. added @lab2_1
  /* offset:272 */ uint64 kernel_satp;
}trapframe;


/*size:32 bytes*/
typedef struct mem_block
{
	uint64 mb_start;          //����ڴ�����ʼ�����ַ
  uint64 mb_size;											//����ڴ���п����õ��ֽ��� �������ڴ��ͷ
	uint64 mb_type;											//����ڴ��ķ������ 1��ʾ�ѱ����� 0��ʾδ������
	struct mem_block* mb_nxt;
}mem_block;

// the extremely simple definition of process, used for begining labs of PKE
typedef struct process_t {
  // pointing to the stack used in trap handling.
  uint64 kstack;
  // user page table
  pagetable_t pagetable;
  // trapframe storing the context of a (User mode) process.
  trapframe* trapframe;
  mem_block* heap_head; // �ڴ������ͷ
  
  uint64 heap_off;	
}process;

// switch to run user app
void switch_to(process*);

// current running process
extern process* current;

// address of the first free page in our simple heap. added @lab2_2
extern uint64 g_ufree_page;

#endif
