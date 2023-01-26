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

// typedef struct mem_block
// {
// 	uint64 mb_size;											//这个内存块的大小
// 	uint64 mb_vm_off;									//这个内存块在虚拟区间中的偏移量
// 	uint64 mb_flag;											//这个内存块的分配情况 1表示已被分配 0表示未被分配
// 	struct mem_block* mb_pre;						
// 	struct mem_block* mb_nxt;
// }mem_block;

// /*表示一个虚拟区间*/
// typedef struct vm_area_struct
// {
// 	uint64 vm_start;										//虚拟内存的首地址
// 	uint64 vm_end;											//虚拟内存的尾地址
//  uint64 vm_type;                    //1表示是HEAP
// 	struct vm_area_struct* vm_nxt;			//指向下一个虚拟区间
//  struct mem_block* vm_mb;           //指向该虚拟区间的第一个mem_block
// }vm_area_struct;


/*size:32 bytes*/
typedef struct mem_block
{
	uint64 mb_start;                    //这个内存块的起始虚拟地址
  uint64 mb_size;											//这个内存块中可利用的字节数 不包含内存块头
	uint64 mb_type;											//这个内存块的分配情况 1表示已被分配 0表示未被分配
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
  //指向已被分配的内存块链表
	mem_block* malloced_mb;   
  //指向未被分配的内存块链表  
  mem_block* free_mb;			
}process;

// switch to run user app
void switch_to(process*);

// current running process
extern process* current;

// address of the first free page in our simple heap. added @lab2_2
extern uint64 g_ufree_page;

#endif
