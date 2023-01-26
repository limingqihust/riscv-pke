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


// type=MB_FREE 插入到free_mb
// type=MB_MALLOCED 插入到malloced_mb
void insert_into_queue(mem_block* p,int type)
{
  if(type==MB_FREE)
  {
    mem_block* now_mb=current->free_mb;
    while(now_mb)
    {
      //插入到链表头
      if(now_mb->mb_size > p->mb_size)
      {
        current->free_mb=p;
        p->mb_nxt=now_mb;
        break;
      }
      //插入到链表尾
      else if(now_mb->mb_nxt==NULL)
      {
        now_mb->mb_nxt=p;
        p->mb_nxt=NULL;
        break;
      }
      else if(now_mb->mb_nxt->mb_size > p->mb_size)
      {
          p->mb_nxt=now_mb->mb_nxt;
          now_mb->mb_nxt=p;
          break;
      }
      now_mb=now_mb->mb_nxt;
    }
  }
  else
  {
    mem_block* now_mb=current->malloced_mb;
    if(current->malloced_mb==NULL)
    {
      current->malloced_mb=(mem_block*)p;
      return ;
    }
    while(now_mb)
    {
      //插入到链表头
      if(now_mb->mb_size > p->mb_size)
      {
        current->malloced_mb=p;
        p->mb_nxt=now_mb;
        break;
      }
      //插入到链表尾
      else if(now_mb->mb_nxt==NULL)
      {
          now_mb->mb_nxt=p;
          p->mb_nxt=NULL;
          break;
      }
      else if(now_mb->mb_nxt->mb_size > p->mb_size)
      {
          p->mb_nxt=now_mb->mb_nxt;
          now_mb->mb_nxt=p;
          break;
      }
      now_mb=now_mb->mb_nxt;
    }
  }
}


// type=MB_FREE 从free_mb中删除
// type=MB_MALLOCED 从malloced_mb删除
void delete_from_queue(mem_block* p,int type)
{
   if(type==MB_FREE) 
   {
    mem_block* now_mb=current->free_mb;
    while(now_mb)
    {
        if(now_mb->mb_nxt==p)
        {
            now_mb->mb_nxt=p->mb_nxt;
            p->mb_nxt=NULL;
        }
        now_mb=now_mb->mb_nxt;
    }
   }
   else
   {
      mem_block* now_mb=current->malloced_mb;
      while(now_mb)
      {
          if(now_mb->mb_nxt==p)
          {
              now_mb->mb_nxt=p->mb_nxt;
              p->mb_nxt=NULL;
          }
          now_mb=now_mb->mb_nxt;
      }
   }
}


// type=MB_FREE 从free_mb中寻找
// type=MB_MALLOCED 从malloced_mb寻找
mem_block* find_mem_block(int need_size,int type)
{
  if(type==MB_FREE)
  {
    mem_block* now_mb=current->free_mb;
    mem_block* avail_mb=NULL;
    /*1 寻找可利用的内存块*/
    while(now_mb)
    {
      if(now_mb->mb_size > need_size)
      {
        avail_mb=now_mb;
        break;
      }
      now_mb=now_mb->mb_nxt;
    }
    return avail_mb;
  }
  else
  {
    return NULL;
  }
}

// 判断一个页虚拟地址所在的页是不是空闲页
// 遍历所有已被分配的内存块 判断这个块是否在这个页中
bool is_free_page(uint64 va)
{
  
  uint64 pa=(uint64)user_va_to_pa((pagetable_t)(current->pagetable), 
                          (void*)va);
  uint64 page_head=ROUNDDOWN(pa,PGSIZE);
  mem_block* now_mb=current->malloced_mb;
  while(now_mb)
  {
    uint64 now_pa=(uint64)user_va_to_pa((pagetable_t)(current->pagetable), 
                                (void*)now_mb->mb_start);
    if(page_head<=now_pa && now_pa<page_head+PGSIZE)
      return false;
    now_mb=now_mb->mb_nxt;
  }
  return true;
}

void print_queue(int type)
{
    mem_block* now_mb=(type==MB_FREE)?current->free_mb:current->malloced_mb;
    while(now_mb)
    {
        sprint("mb_size:%llx,mb_start:%llx,mb_pa_start:%llx\n",now_mb->mb_size,now_mb->mb_start,
              (uint64)user_va_to_pa((pagetable_t)(current->pagetable),(void*)now_mb->mb_start));
        now_mb=now_mb->mb_nxt;
    }
}






//
// maybe, the simplest implementation of malloc in the world ... added @lab2_2
//
uint64 sys_user_allocate_page(int n) {
  // void* pa = alloc_page();
  // uint64 va = g_ufree_page;
  // g_ufree_page += PGSIZE;
  // user_vm_map((pagetable_t)current->pagetable, va, PGSIZE, (uint64)pa,
  //        prot_to_type(PROT_WRITE | PROT_READ, 1));
  int need_size=n+sizeof(mem_block);
  if(need_size>PGSIZE)
    panic("malloc bytes is too large\n");
  /*0 初始化*/
  // if(current->free_mb==NULL)
  // {
  //   void* pa = alloc_page();
  //   uint64 va = g_ufree_page;
  //   g_ufree_page += PGSIZE;
  //   user_vm_map((pagetable_t)current->pagetable, va, PGSIZE, (uint64)pa,
  //         prot_to_type(PROT_WRITE | PROT_READ, 1));
  //   current->free_mb=(mem_block*)pa;
  //   current->free_mb->mb_start=va;
  //   current->free_mb->mb_size=PGSIZE-sizeof(mem_block);
  //   current->free_mb->mb_type=MB_FREE;
  //   current->free_mb->mb_pre=NULL;
  //   current->free_mb->mb_nxt-NULL;
  // }

  mem_block* avail_mb=NULL;
  /*1 寻找可利用的内存块*/
  avail_mb=find_mem_block(need_size,MB_FREE);
  /*2.1 找到可利用的内存块*/
  if(avail_mb!=NULL)
  {
    uint64 va=avail_mb->mb_start;
    mem_block* nxt_mb=(mem_block*)(avail_mb+sizeof(mem_block)+n);
    nxt_mb->mb_start=avail_mb->mb_start + n + sizeof(mem_block);
    nxt_mb->mb_size=avail_mb->mb_size - n - sizeof(mem_block);
    nxt_mb->mb_type=MB_FREE;
    /*将这个内存块插入到空闲链表*/
    insert_into_queue(nxt_mb,MB_FREE);
    avail_mb->mb_size=n;
    avail_mb->mb_type=MB_MALLOCED;
    /*将avail_mb从MB_FREE链表中删除*/
    delete_from_queue(avail_mb,MB_FREE);
    /*将avail_mb加入到MB_MALLOCED链表中*/
    insert_into_queue(avail_mb,MB_MALLOCED);
    /*将nxt_mb加入到MB_FREE链表中*/
    insert_into_queue(nxt_mb,MB_FREE);
    return va;
  }

  /*2.2 没有找到可利用的内存块*/
  else
  {
    sprint("there is no avail_mb\n");
    void* pa = alloc_page();
    uint64 va = g_ufree_page;
    g_ufree_page += PGSIZE;
    user_vm_map((pagetable_t)current->pagetable, va, PGSIZE, (uint64)pa,
          prot_to_type(PROT_WRITE | PROT_READ, 1));
    avail_mb=(mem_block*)pa;
    avail_mb->mb_start=va;
    avail_mb->mb_size=n;
    avail_mb->mb_type=MB_MALLOCED;
    avail_mb->mb_nxt=NULL;
    insert_into_queue(avail_mb,MB_MALLOCED);    
  
    mem_block* nxt_mb=(mem_block*)(pa+sizeof(mem_block)+n);
    nxt_mb->mb_start=va+n;
    nxt_mb->mb_size=PGSIZE-sizeof(mem_block)*2-n;
    nxt_mb->mb_type=MB_FREE;
    nxt_mb->mb_nxt=NULL;
    insert_into_queue(nxt_mb,MB_FREE);
    return va;
  }
}

//
// reclaim a page, indicated by "va". added @lab2_2
//
uint64 sys_user_free_page(uint64 va) {
  mem_block* now_mb=current->malloced_mb;
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
