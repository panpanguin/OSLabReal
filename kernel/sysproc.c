#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

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


  argint(0, &n);
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
int
sys_pgaccess(void)
{
  // lab pgtbl: your code here.
  uint64 bufptr;
  int npage;
  uint64 bitptr;
  uint64 bitmap = 0;
  int count = 0;
  struct proc *p = myproc();
  //parsing the arguments
  argaddr(0, &bufptr);
  argint(1, &npage);
  argaddr(2, &bitptr);
  //setting an upper limit on npage
  if(npage > 64) return -1;
  
  for(uint64 i = bufptr; i < bufptr + npage * PGSIZE; i += PGSIZE)
  {
    //Finding the right pte
    pte_t *pte = walk(p->pagetable, i, 0);
    
    if(*pte & PTE_A)
    {
      bitmap = bitmap | 1 << count;
      //clear PTA after checking
      *pte = (*pte) & (~PTE_A);
      printf("%p ", *pte);
    }
    else printf("%p ", *pte);
    count++;
  }
  printf("\n");
  
  //copy the bitmask to user
  copyout(p->pagetable, bitptr, (char*)&bitmap, sizeof(bitmap));
  
  return 0;
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
