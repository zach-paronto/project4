#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "wmap.h"
#include "llnode.h"

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
        addrCopy += PGSIZE;
        lenCopy -= PGSIZE;
    }

    return addr;
}