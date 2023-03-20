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

//将虚拟地址[left,right]映射到物理内存
void* increase_vm(pagetable_t page_dir,uint64 left,uint64 right)
{
//    sprint("increase_vm:map [%llx,%llx] to memory\n",left,right);
    uint64 old_page_off=((left & 0xfff)==0)?left:((left | 0xfff) + 1);
    if(right<=old_page_off)
        return user_va_to_pa(page_dir, (void*)left);
    uint64 new_page_off=right | 0xfff;
    for(uint64 cur=old_page_off;cur<=new_page_off;cur+=PGSIZE)
    {
        void* pa = alloc_page();
//        sprint("    map [%llx,%llx] to [%llx,%llx]\n",cur,cur+PGSIZE-1,(uint64)pa,(uint64)pa+PGSIZE-1);
        user_vm_map(page_dir, cur, PGSIZE, (uint64)pa,
                prot_to_type(PROT_WRITE | PROT_READ, 1));
    }
    return user_va_to_pa(page_dir, (void*)left);
}

void print_mem_block()
{
    sprint("print_mem_block:\n");
    mem_block* cur_mb=current->heap_head;
    while(cur_mb)
    {
        sprint("    mb_start:%llx mb_size:%llx mb_type:%d mb_physical_off:%llx\n",
               cur_mb->mb_start,cur_mb->mb_size,cur_mb->mb_type,(uint64)cur_mb);
        cur_mb=cur_mb->mb_nxt;
    }
}

//
// maybe, the simplest implementation of malloc in the world ... added @lab2_2
//
uint64 sys_user_allocate_page(int n) 
{
//    sprint("malloc byte %d\n",n);
  mem_block* cur_mb=current->heap_head;
//  sprint("find available mem_block\n");
  while(cur_mb)
  {
    // find availalbe mem_block
    if(cur_mb->mb_type==MB_FREE && cur_mb->mb_size>=n)
    {
      cur_mb->mb_type=MB_MALLOCED;
      return cur_mb->mb_start;
    }
    cur_mb=cur_mb->mb_nxt;
  }
//  sprint("need a new mem_block\n");
  // alloc a new mem_block
  uint64 old_heap_off=current->heap_off;
  uint64 new_heap_off=old_heap_off+n+sizeof(mem_block);
//  sprint("old_heap_off is %llx\nnew_heap_off is %llx\n",old_heap_off,new_heap_off);

  // 将[old_heap_off,new_heap_off-1]的虚拟内存映射到物理内存
  mem_block* pa;
  pa=increase_vm(current->pagetable,old_heap_off,new_heap_off-1);

//  sprint("the new mem_block is %llx\n",(uint64)pa);

  pa->mb_start=old_heap_off+sizeof(mem_block);

  pa->mb_size=n;
  pa->mb_type=MB_MALLOCED;
  pa->mb_nxt=NULL;

  if(current->heap_head==NULL)
      current->heap_head=pa;
  else {
      cur_mb = current->heap_head;
      while (cur_mb) {
          if (cur_mb->mb_nxt == NULL) {
              cur_mb->mb_nxt = pa;
              break;
          }
          cur_mb = cur_mb->mb_nxt;
      }
  }

  current->heap_off=new_heap_off;
  return pa->mb_start;
}

//
// reclaim a page, indicated by "va". added @lab2_2
//
uint64 sys_user_free_page(uint64 va) {
  mem_block* cur_mb=current->heap_head;
  while(cur_mb)
  {
      if(cur_mb->mb_start==va)
      {
          cur_mb->mb_type=MB_FREE;
          break;
      }
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
      return sys_user_allocate_page(ROUNDUP(a1,32));
    case SYS_user_free_page:         
      return sys_user_free_page(a1);
    default:
      panic("Unknown syscall %ld \n", a0);
  }
}
