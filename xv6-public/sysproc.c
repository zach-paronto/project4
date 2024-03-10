#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "wmap.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int
sys_wmap(void)
{
  uint addr;
  int length, flags, fd;
  if (argint(0, (int *)&addr) < 0 || argint(1, &length) < 0 || argint(2,  &flags) < 0 || argint(3, &fd) < 0 || argint(3, &fd) >= NOFILE) {
    return FAILED;
  }
  return wmap(addr, length, flags, fd);
}

int
sys_wunmap(void)
{
  uint addr;
  if (argint(0, (int *)&addr) < 0) {
    return FAILED;
  }
  return wunmap(addr);
}

int
sys_wremap(void)
{
  uint oldaddr;
  int oldsize, newsize, flags;
  if (argint(0, (int *)&oldaddr) < 0 || argint(1, &oldsize) < 0 || argint(2, &newsize) < 0 || argint(3, &flags) < 0) {
    return FAILED;
  }
  return wremap(oldaddr, oldsize, newsize, flags);
}

int 
sys_getwmapinfo(void)
{
  struct wmapinfo* wminfo;
  if (argptr(0, (void *)&wminfo, sizeof(wminfo)) < 0) {
    return FAILED;
  }
  return getwmapinfo(wminfo);
}

int
sys_getpgdirinfo(void)
{
  struct pgdirinfo* pdinfo;
  if (argptr(0, (void *)&pdinfo, sizeof(pdinfo)) < 0) {
    return FAILED;
  }
  return getpgdirinfo(pdinfo);
}