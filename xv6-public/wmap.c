#include "wmap.h"
#include "types.h"
#include "mmu.h"
#include "proc.h"
#include "defs.h"
#include "memlayout.h"

// To Do: file-backed mapping in wmap, lazy allocation, wunmap, wremap, test functions, modified sys calls.

/*
From your working directory, run
    ~cs537-1/tests/P4/runtests

If you want to run all the tests despite any failures, use
    ~cs537-1/tests/P4/runtests -c

List of other Testing Scripts:
    ~cs537-1/tests/P4/runtests starter
    ~cs537-1/tests/P4/runtests wmap
    ~cs537-1/tests/P4/runtests wunmap
    ~cs537-1/tests/P4/runtests fork
    ~cs537-1/tests/P4/runtests wremap
    */

typedef struct node
{
    uint addr;
    int length;
    int flags;
    int fd;
    struct node *next;
} node;

node *head = 0;


void insert_node(struct proc *process, node *new)
{
    if (head == 0)
    {
        head = new;
        return;
    }

    node *curr = head;
    node *prev = 0;
    while (curr != 0)
    {
        if (curr->addr > new->addr)
        {
            if (prev == 0)
            {
                new->next = curr;
                head = new;
                return;
            }
            prev->next = new;
            new->next = curr;
            return;
        }
        prev = curr;
        curr = curr->next;
    }
    prev->next = new;
}

void remove(struct proc *process, uint addr)
{
    node *curr = head;
    node *prev = 0;
    while (curr != 0)
    {
        if (curr->addr == addr)
        {
            if (prev == 0)
            {
                head = curr->next;
                kfree((char *)curr);
                return;
            }
            prev->next = curr->next;
            kfree((char *)curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

node *find_node(struct proc *process, uint addr)
{
    node *curr = head;
    while (curr != 0)
    {
        if (curr->addr == addr)
        {
            return curr;
        }
        curr = curr->next;
    }
    return 0;
}

// helper function to find the first free space of a given size in the linked list
node *find_free_space(struct proc *process, int length)
{
    node *curr = head;
    node *prev = 0;
    while (curr != 0)
    {
        if (prev != 0)
        {
            if (curr->addr - (prev->addr + prev->length) >= length)
            {
                node *new = (node *)kalloc();
                new->addr = PGROUNDUP(prev->addr + prev->length);
                new->length = length;
                new->flags = MAP_ANONYMOUS;
                new->fd = -1;
                new->next = 0;
                return new;
            }
        }
        prev = curr;
        curr = curr->next;
    }
    return 0;
}

int is_conflict(struct proc *process, uint addr, int length)
{
    node *curr = head;
    while (curr != 0)
    {
        if (curr->addr <= addr && curr->addr + curr->length >= addr)
        {
            return 1;
        }
        if (curr->addr >= addr && curr->addr <= addr + length)
        {
            return 1;
        }
        {
            return 1;
        }
        curr = curr->next;
    }
    return 0;
}

// just frees the list of nodes for a given process
void free_all(struct proc *process)
{
    node *curr = head;
    while (curr != 0)
    {
        node *temp = curr;
        curr = curr->next;
        kfree((char *)temp);
    }
}

uint wmap(uint addr, int length, int flags, int fd)
{
    if (addr > KERNBASE || addr < 0x60000000 || ((addr & (PGSIZE - 1)) != 0)
        || length <= 0
        || flags < 0)
    {
        return FAILED;
    }
    struct proc *process = myproc();
    int lenCopy = length;
    uint addrCopy = addr;
    char *mem;

    if ((flags & MAP_FIXED) == MAP_FIXED)
    {
        if (is_conflict(process, addr, PGROUNDUP(addr+lenCopy)))
        {
            return FAILED;
        }
    }
    else
    {
        node *free = find_free_space(process, lenCopy);
        if (free == 0)
        {
            return FAILED;
        }
        addrCopy = free->addr;
        addr = addrCopy;
        kfree((char *)free);
    }

    while (lenCopy > 0)
    {
        mem = kalloc();
        if (mem == 0)
        {
            return FAILED;
        }
        mappages(process->pgdir, &addrCopy, 4096, V2P(mem), PTE_W | PTE_U);
        insert(process, addrCopy, PGSIZE, flags, fd);
        addrCopy += PGSIZE;
        lenCopy -= PGSIZE;
    }

    return addr;
}

uint wremap(uint oldaddr, int oldsize, int newsize, int flags) {
    return SUCCESS;
}

int wunmap(uint addr)
{
    struct proc *curproc = myproc();
    if (addr < 0 || addr > KERNBASE || ((addr & (PGSIZE - 1)) != 0))
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