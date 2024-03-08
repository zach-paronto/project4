#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "wmap.h"

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

void insert_node(struct proc *process, node *new)
{
    if (process->head == 0)
    {
        process->head = new;
        return;
    }

    node *curr = process->head;
    node *prev = 0;
    while (curr != 0)
    {
        if (curr->addr > new->addr)
        {
            if (prev == 0)
            {
                new->next = curr;
                process->head = new;
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

// helper function to insert a node into the linked list
// will be sorted by addr in ascending order,
// and the space between two nodes is free
void insert(struct proc *process, uint addr, int length, int flags, int fd)
{
    node *new = (node *)kalloc();
    new->addr = addr;
    new->length = length;
    new->flags = flags;
    new->fd = fd;
    new->next = 0;

    if (process->head == 0)
    {
        process->head = new;
        return;
    }

    node *curr = process->head;
    node *prev = 0;
    while (curr != 0)
    {
        if (curr->addr > addr)
        {
            if (prev == 0)
            {
                new->next = curr;
                process->head = new;
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

// helper function to remove a node from the linked list
void remove(struct proc *process, uint addr)
{
    node *curr = process->head;
    node *prev = 0;
    while (curr != 0)
    {
        if (curr->addr == addr)
        {
            if (prev == 0)
            {
                process->head = curr->next;
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

// helper function to find a node in the linked list
node *find(struct proc *process, uint addr)
{
    node *curr = process->head;
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
// returns null if space cannot be found
node *find_free(struct proc *process, int length)
{
    node *curr = process->head;
    node *prev = 0;
    while (curr != 0)
    {
        if (prev != 0)
        {
            if (curr->addr - (prev->addr + prev->length) >= length)
            {
                node *new = (node *)kalloc();
                new->addr = prev->addr + prev->length;
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
    node *curr = process->head;
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
    node *curr = process->head;
    while (curr != 0)
    {
        node *temp = curr;
        curr = curr->next;
        kfree((char *)temp);
    }
}

// wmap will map a file into the address space of the calling process
// addr is the address at which the file should be mapped or a hint to where, depending on flags
// length is the length of memory to be allocated
// flags correspond to the wmap flags in wmap.h
// fd is the file descriptor of the file to be mapped. can be ignored if MAP_ANONYMOUS is set
// returns the address at which the file was mapped
uint wmap(uint addr, int length, int flags, int fd)
{
    if (addr > KERNBASE || addr < 0x60000000)
    {
        return FAILED;
    }
    if (length < 0)
    {
        return FAILED;
    }
    if (flags < 0)
    {
        return FAILED;
    }

    struct proc *process = myproc();
    int lenCopy = length;
    int numPages = 0;
    uint addrCopy = addr;
    char *mem;

    // if we are in FIXED mode, we must use the listed address with the length specified.
    // if that range is not available, return FAILED
    if ((flags & MAP_FIXED) == MAP_FIXED)
    {
        if (is_conflict(process, addr, length))
        {
            return FAILED;
        }
    }
    else
    {
        node *free = find_free(process, length);
        if (free == 0)
        {
            return FAILED;
        }
        addrCopy = free->addr;
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
        numPages++;
        addrCopy += PGSIZE;
        lenCopy -= PGSIZE;
    }

    return addr;
}