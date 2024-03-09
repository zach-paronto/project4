#include "types.h"
#include "defs.h"
#include "wmap.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "llnode.h"

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
