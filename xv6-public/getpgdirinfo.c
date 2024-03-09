#include "types.h"
#include "defs.h"
#include "wmap.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "llnode.h"


int getpgdirinfo(struct pgdirinfo *pdinfo){
    struct proc *curproc = myproc();
    pdinfo->n_upages = 0;
    if (curproc == 0)
    {
        return FAILED;
    }
    node *curr = curproc->head;
    int i = 0;
    while (curr != 0)
    {
        if (curproc->pgdir[i / PGSIZE] & PTE_U) { 
            pdinfo->va[pdinfo->n_upages] = curr->addr;
            pdinfo->pa[pdinfo->n_upages] = PTE_ADDR(*walkpgdir(curproc->pgdir, (void *)curr->addr, 0));
            pdinfo->n_upages++;
            i++;
            curr = curr->next;
        }
    }
    return SUCCESS;
}