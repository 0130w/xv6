#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);  // get the first argument of syscall
  if(n < 0)
    n = 0;

  #ifdef LAB_TRAPS
  backtrace();
  #endif
  
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
uint64
sys_pgaccess(void)
{
  int page_num;
  uint64 base;
  uint64 abits_addr;
  argaddr(0, &base);
  argint(1, &page_num);
  argaddr(2, &abits_addr);
  // set upper bound of the number of pages
  if(page_num > 32) {
    printf("warning:  \
                  not support to check page numbers greater than 32   \
                  check 32 pages instead.\n");
    page_num = 32;
  }
  unsigned int abits = 0;
  for(int i = 0; i < page_num; ++i) {
    uint64 addr = base + i * PGSIZE;
    for(int offset = 0; offset < PGSIZE; ++offset) {
      uint64 va = addr | offset;
      pte_t *pte = walk(myproc()->pagetable, va, 0);
      if(pte == 0)
        return -1;
      if((*pte) & PTE_A) {
        abits |= (1 << i);
        (*pte) &= (~PTE_A);
        break;
      }
      (*pte) &= (~PTE_A);
    }
  }
  return (uint64)(copyout(myproc()->pagetable, abits_addr, (char *)&abits, sizeof(abits)));
}
#endif

#ifdef LAB_TRAPS
uint64
sys_sigalarm(void)
{
  int ticks;
  uint64 handler;
  argint(0, &ticks);
  argaddr(1, &handler);
  myproc()->alarm_interval = ticks;
  // handler is virtual address of handler function in user space
  myproc()->alarm_handler = handler;
  return 0;
}

uint64
sys_sigreturn(void)
{
  myproc()->ticks = 0;
  restore_alarm_state(myproc());
  return myproc()->trapframe->a0; 
}
#endif

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// return tracing syscall number if success
// -1 if error
uint64
sys_trace(void)
{
  int mask;
  argint(0, &mask);
  if(mask <= 0)
    return -1;
  myproc()->trace_mask |= mask;
  return 0;
}

uint64
sys_sysinfo(void)
{
  struct sysinfo info;
  info.freemem = calculate_free();
  info.nproc = proc_number();
  uint64 p;
  argaddr(0, &p);
  return (uint64)(copyout(myproc()->pagetable, p, (char *)&info, sizeof(info)));
}