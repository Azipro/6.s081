// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

struct {
  struct spinlock lock;
  uint64 start; // first page starts from PGROUNDUP((uint64)end)
  int pageusecount[NPAGES];
} kmemcount;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  int i = 0;
  p = (char*)PGROUNDUP((uint64)pa_start);
  kmemcount.start = PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE){
    acquire(&kmemcount.lock);
    kmemcount.pageusecount[i ++] = 0;
    release(&kmemcount.lock);
    kfree(p);
  }
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;
  int pgn;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  pgn = ((uint64)pa - kmemcount.start) / PGSIZE;
  acquire(&kmemcount.lock);
  if(-- kmemcount.pageusecount[pgn] > 0){
    release(&kmemcount.lock);
    return ;
  }
  release(&kmemcount.lock);

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;
  int pgn;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r){
    memset((char*)r, 5, PGSIZE); // fill with junk
    pgn = ((uint64)r - kmemcount.start) / PGSIZE;
    acquire(&kmemcount.lock);
    kmemcount.pageusecount[pgn] = 1;
    release(&kmemcount.lock);
  }
  return (void*)r;
}

void kcountsub(uint64 pa){
  int pgn = ((uint64)pa - kmemcount.start) / PGSIZE;
  acquire(&kmemcount.lock);
  -- kmemcount.pageusecount[pgn];
  release(&kmemcount.lock);
}

void kcountadd(uint64 pa){
  int pgn = ((uint64)pa - kmemcount.start) / PGSIZE;
  acquire(&kmemcount.lock);
  ++ kmemcount.pageusecount[pgn];
  release(&kmemcount.lock);
}

void acquirekmemclock(){
  acquire(&kmemcount.lock);
}

void releasekmemclock(){
  release(&kmemcount.lock);
}

int kcountall(uint64 pa){
  int pgn = ((uint64)pa - kmemcount.start) / PGSIZE;
  return kmemcount.pageusecount[pgn];
}
