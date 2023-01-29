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
#include "sched.h"

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
  // reclaim the current process, and reschedule. added @lab3_1
  free_process( current );
  schedule();
  return 0;
}

//
// maybe, the simplest implementation of malloc in the world ... added @lab2_2
//
uint64 sys_user_allocate_page() {
  void* pa = alloc_page();
  /*alloc a page of 4kb to va*/
  uint64 va = g_ufree_page;
  g_ufree_page += PGSIZE;
  user_vm_map((pagetable_t)current->pagetable, va, PGSIZE, (uint64)pa,
         prot_to_type(PROT_WRITE | PROT_READ, 1));

  return va;
}

//
// reclaim a page, indicated by "va". added @lab2_2
//
uint64 sys_user_free_page(uint64 va) {
  user_vm_unmap((pagetable_t)current->pagetable, va, PGSIZE, 1);
  return 0;
}

//
// kerenl entry point of naive_fork
//
ssize_t sys_user_fork() {
  sprint("User call fork.\n");
  return do_fork( current );
}

//
// kerenl entry point of yield. added @lab3_2
//
ssize_t sys_user_yield() {
  // TODO (lab3_2): implment the syscall of yield.
  // hint: the functionality of yield is to give up the processor. therefore,
  // we should set the status of currently running process to READY, insert it in
  // the rear of ready queue, and finally, schedule a READY process to run.

  // sprint("current->pid=%d\n",current->pid);
  // sprint("current->queue_next->pid=%d\n",current->queue_next->pid);
  /*set the status of currently running process to READY*/
  current->status=READY;
  /*insert current process in the rear of ready queue*/
  insert_to_ready_queue(current);
  /*schedule a READY process to run*/
  schedule();

  return 0;
}


/*申请一个信号灯 返回这个信号灯的序列号*/
int sys_user_sem_new(int n)
{
  int i;
  for(i=0;i<NSEM;i++)
  {
    if(sem[i].num==-1)
      break;
  }
  if(sem[i].num!=-1)
    panic("need more semaphore\n");
  sem[i].num=n;
  sem[i].wait_semaphore_process_queue=NULL;
  return i;
}

// 对n号信号灯进行P操作
// 如果num大于0 信号灯数量减一
// 如果num==0 将该进程加入到信号灯等待序列 并调度其他进程运行
void sys_user_sem_P(int n)
{
  // sprint("process[%lld] P semaphore[%d],semaphore[%d] has num %d\n",current->pid,n,n,sem[n].num);
  if(sem[n].num>0)
  {
    sem[n].num--;
  }
  else
  {
    // sprint("process[%lld] waiting for semaphore[%d] and insert to sem_waiting queue\n",current->pid,n);
    insert_to_semaphore_wait_queue(current,n);
    schedule();
  }
}

// 对信号灯进行V操作
// 如果信号灯数量为0 从该信号灯的等待队列中取出一个加入到ready_queue
void sys_user_sem_V(int n)
{
  sem[n].num++;
  // 如果这个信号灯的数量之前为0且该信号灯的等待队列有进程 将该进程加入到等待队列
  // sprint("process[%lld] V semaphore[%d],semaphore[%d] has num %d\n",current->pid,n,n,sem[n].num);
  if(sem[n].num==1)
  {
    process* new_proc=schedule_semaphore(n);
    if(new_proc!=NULL)
    {
      sem[n].num--;
      insert_to_ready_queue(new_proc);
    }
  }
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
      return sys_user_allocate_page();
    case SYS_user_free_page:
      return sys_user_free_page(a1);
    case SYS_user_fork:
      return sys_user_fork();
    case SYS_user_yield:
      return sys_user_yield();
    case SYS_user_sem_new:
      return sys_user_sem_new(a1);
    case SYS_user_sem_P:
      sys_user_sem_P(a1);
      return 0;
     case SYS_user_sem_V:
      sys_user_sem_V(a1);
      return 0;
    default:
      panic("Unknown syscall %ld \n", a0);
  }
}
