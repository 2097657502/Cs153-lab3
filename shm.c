#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct shm_page {
    uint id;
    char *frame;
    int refcnt;
  } shm_pages[64];
} shm_table;

void shminit() {
  int i;
  initlock(&(shm_table.lock), "SHM lock");
  acquire(&(shm_table.lock));
  for (i = 0; i< 64; i++) {
    shm_table.shm_pages[i].id =0;
    shm_table.shm_pages[i].frame =0;
    shm_table.shm_pages[i].refcnt =0;
  }
  release(&(shm_table.lock));
}

int shm_open(int id, char **pointer) {

        int i;
//you write this
	acquire(&(shm_table.lock)); //prevent race condition
	struct proc* cp=myproc();
	//here is to find the id in shm table
	//64 is size of shm pages
	for(i=0; i<64; i++){ 
		//if id match
		if(shm_table.shm_pages[i].id==id){
			//map VA to PA
			mappages(cp->pgdir, (char*)PGROUNDUP(cp->sz), PGSIZE, V2P(shm_table.shm_pages[i].frame), PTE_W|PTE_U);
			//increase process that is using counter
			//process accessing counter
			shm_table.shm_pages[i].refcnt++;
			//return pointer to VA
			*pointer=(char*)PGROUNDUP(cp->sz);
			//update sz pagesize
			cp->sz=PGROUNDUP(cp->sz+PGSIZE);
		}
		// if id not match
		else{
			//id to VA
			shm_table.shm_pages[i].id=id;
			//reset
			shm_table.shm_pages[i].frame=kalloc();
			memset(shm_table.shm_pages[i].frame,0,PGSIZE);
			shm_table.shm_pages[i].refcnt=1;
			mappages(cp->pgdir, (char*)PGROUNDUP(cp->sz), PGSIZE, V2P(shm_table.shm_pages[i].frame), PTE_W|PTE_U);
			*pointer=(char*)PGROUNDUP(cp->sz);
			cp->sz=PGROUNDUP(cp->sz+PGSIZE);
		}
	}
	release(&(shm_table.lock));
	
	return 0; //added to remove compiler warning -- you should decide what to return
}


int shm_close(int id) {
  int i;
  acquire(&(shm_table.lock)); //prevent the race condition
  for(i = 0; i < 64; i++){
    if(shm_table.shm_pages[i].refcnt != 0){ //Checking to see if the refcnt of the current page is 0
	if(shm_table.shm_pages[i].id==id){  //If not, we check to see if the id is the same as the one we passed in
	    shm_table.shm_pages[i].refcnt--;//If it is, this means that we have a reference to this page, decrement
	}
    }
    else{ //Clearing the page table if the refcnt of one of thesep pages is 0
	shm_table.shm_pages[i].id =0;
        shm_table.shm_pages[i].frame =0;
        shm_table.shm_pages[i].refcnt =0;
    }

  }
  release(&(shm_table.lock));
  




return 0; //added to remove compiler warning -- you should decide what to return
}
