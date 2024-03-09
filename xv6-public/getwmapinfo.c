#include "types.h"
#include "defs.h"
#include "wmap.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "llnode.h"

int getwmapinfo(struct wmapinfo *wmpinfo){
    struct proc *curproc = myproc();
    wmpinfo->total_mmaps = 0;
    if (curproc == 0)
    {
        return FAILED;
    }
    node *curr = curproc->head;
    while (curr != 0)
    {
        wmpinfo->total_mmaps++;
        wmpinfo->addr[wmpinfo->total_mmaps] = curr->addr;
        wmpinfo->length[wmpinfo->total_mmaps] = curr->length;
        wmpinfo->n_loaded_pages[wmpinfo->total_mmaps] = curr->length / PGSIZE;
    }
    return SUCCESS;
}