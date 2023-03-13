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
#include "pmm.h"
#include "vmm.h"
#include "spike_interface/spike_utils.h"


#define true 1
#define false 0
//
// implement the SYS_user_print syscall
//
ssize_t sys_user_print(const char* buf, size_t n) {
  // buf is now an address in user space of the given app's user stack,
  // so we have to transfer it into phisical address (kernel is running in direct mapping).
  assert( current );
  char* pa = (char*)user_va_to_pa((pagetable_t)(current->pagetable), (void*)buf);
  sprint(pa);
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

//将虚拟地址[old_heap_off,new_heap_off]映射到物理内存
void increase_vm(pagetable_t page_dir,uint64 old_heap_off,uint64 new_heap_off)
{

}

//
// maybe, the simplest implementation of malloc in the world ... added @lab2_2
//
uint64 sys_user_allocate_page(int n) 
{
  mem_block* cur_mb=current->heap_head;
  while(cur_mb)
  {
    // 找到一个合适的mem_block
    if(cur_mb->mb_type==MB_FREE && cur_mb->mb_size>=n)
    {
      cur_mb->mb_type=MB_MALLOCED;
      return cur_mb->mb_start;
    }
    cur_mb=cur_mb->mb_nxt;
  }
  // 需要一个新的mem_block
  uint64 old_heap_off=current->heap_off;
  uint64 new_heap_off=old_heap_off+n+sizeof(mem_block);
  increase_vm(current->pagetable,old_heap_off,new_heap_off);
  

}

//
// reclaim a page, indicated by "va". added @lab2_2
//
uint64 sys_user_free_page(uint64 va) {
  mem_block* now_mb=current->heap_head;
  while(now_mb)
  {
    if(now_mb->mb_start==va)
    {
      break;
    }
    now_mb=now_mb->mb_nxt;
  }
  delete_from_queue(now_mb,MB_MALLOCED);
  /*如果整个页都空闲*/
  if(is_free_page(va))
  {
    user_vm_unmap((pagetable_t)current->pagetable, ROUNDDOWN(va,PGSIZE), PGSIZE, 1);
  }
  else
  {
    now_mb->mb_type=MB_FREE;
    insert_into_queue(now_mb,MB_FREE);
  }
  return 0;
}

//
// [a0]: the syscall number; [a1] ... [a7]: arguments to the syscalls.
// returns the code of success, (e.g., 0 means success, fail for otherwise)
//
long do_syscall(long a0, long a1, long a2, long a3, long a4, long a5, long a6, long a7) {
  switch (a0) {
    case SYS_user_print:
      return sys_user_print((const char*)a1, a2);
    case SYS_user_exit:
      return sys_user_exit(a1);
    // added @lab2_2
    case SYS_user_allocate_page:
      return sys_user_allocate_page(a1);
    case SYS_user_free_page:         
      return sys_user_free_page(a1);
    default:
      panic("Unknown syscall %ld \n", a0);
  }
}
