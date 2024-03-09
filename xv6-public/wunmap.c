#include "types.h"
#include "defs.h"
#include "wmap.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "llnode.h"

int wunmap(uint addr)
{
    struct proc *curproc = myproc();
    if (addr < 0 || addr > KERNBASE)
    {
        return FAILED;
    }
    node *node_to_remove;
    if( (node_to_remove = find(curproc, addr)) == 0)
    {
        return FAILED;
    }

    int lenCopy = node_to_remove->length;
    uint addrCopy = addr;
    remove(curproc, addr);

    while (lenCopy > 0)
    {
        pte_t *pte = walkpgdir(curproc->pgdir, &addrCopy, 0);
        if (pte == 0)
        {
            return FAILED;
        }
        uint physical_address = PTE_ADDR(*pte);
        kfree(P2V(physical_address));
        addrCopy += PGSIZE;
        lenCopy -= PGSIZE;

    }

    return SUCCESS;
}