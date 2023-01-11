/**
 * virtmem.c 
 */

#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define TLB_SIZE 16
#define PAGES 1024
#define PAGE_MASK 1023/* TODO */

#define PAGE_SIZE 1024
#define OFFSET_BITS 10
#define OFFSET_MASK 1023/* TODO */

#define FRAMES 256
#define LOGICAL_MEMORY_SIZE  PAGES * PAGE_SIZE
#define PHYSICAL_MEMORY_SIZE FRAMES * PAGE_SIZE

// Max number of characters per line of input file to read.
#define BUFFER_SIZE 10

int p = 0; //mode of the program
int fifo_next = 0; //denotes next frame for FIFO

//LRU PART 
//created for the LRU replacement, not related to FIFO
struct lru_node {
  int age_val;
  int data;
  struct lru_node *next;
};

typedef struct lru_node LRU_node;

int lru_size = 0;
LRU_node *lru_head;
//LRU PART END

struct tlbentry {
  int logical;
  int physical;
};

// TLB is kept track of as a circular array, with the oldest element being overwritten once the TLB is full.
struct tlbentry tlb[TLB_SIZE];
// number of inserts into TLB that have been completed. Use as tlbindex % TLB_SIZE for the index of the next TLB line to use.
int tlbindex = 0;

// pagetable[logical_page] is the physical page number for logical page. Value is -1 if that logical page isn't yet in the table.
int pagetable[PAGES];

signed char main_memory[PHYSICAL_MEMORY_SIZE];

// Pointer to memory mapped backing file
signed char *backing;

int max(int a, int b){
  if (a > b)
    return a;
  return b;
}

/* Returns the physical address from TLB or -1 if not present. */
int search_tlb(int logical_page) {
    /* TODO */
    for(int i=0; i<TLB_SIZE; i++){
	if(tlb[i].logical == logical_page){
	    return tlb[i].physical;
	}
    }
    return -1;
}

/* Adds the specified mapping to the TLB, replacing the oldest mapping (FIFO replacement). */
void add_to_tlb(int logical, int physical) {
    /* TODO */
   int index = tlbindex % TLB_SIZE;
   tlb[index].physical = physical;
   tlb[index].logical = logical;
   tlbindex++;
}

int FIFO(){
  for(int i =0; i<PAGES; i++){
    if(pagetable[i] == fifo_next){
        return i;
    }
  }
  return -1;
}

int LRU(){
  LRU_node *curr = lru_head;
  LRU_node *prev = lru_head;

  int min = 0;

  while(curr->next != NULL){
      int next_age_val = (curr->next)->age_val;

      if(min < next_age_val){
          prev = curr;
          min = next_age_val;
      }
      curr = curr->next;
  }

  LRU_node *tmp1 = (prev->next);
  LRU_node *tmp2 = tmp1 ->next;

  int res = (tmp1)->data;
  free(tmp1);

  prev->next = tmp2;
  lru_size--;

  return res;
}

int main(int argc, const char *argv[]){
      if (argc != 5)
      {
          fprintf(stderr, "Usage ./virtmem backingstore input -p 0/1 (0 for FIFO, 1 for LRU)\n");
          exit(1);
      }

      p = atoi(argv[4]);
      
      //LRU is initialized to keep track of LRU replacement
      lru_head = malloc(sizeof(LRU_node));
      lru_head->age_val = 0;
      lru_head->next = NULL;
      
      const char *backing_filename = argv[1]; 
      int backing_fd = open(backing_filename, O_RDONLY);
      backing = mmap(0, LOGICAL_MEMORY_SIZE, PROT_READ, MAP_PRIVATE, backing_fd, 0); 
      
      const char *input_filename = argv[2];
      FILE *input_fp = fopen(input_filename, "r");
      
      // Fill page table entries with -1 for initially empty table.
      int i;
      for (i = 0; i < PAGES; i++) {
        pagetable[i] = -1;
      }
      
      // Character buffer for reading lines of input file.
      char buffer[BUFFER_SIZE];
      
      // Data we need to keep track of to compute stats at end.
      int total_addresses = 0;
      int tlb_hits = 0;
      int page_faults = 0;
      
      // Number of the next unallocated physical page in main memory
      int free_page = FRAMES;
     
      // Declaring pointers for the further memcpy call.
      signed char *transfer_location_in_main_memory = 0;
      signed char *data_location_in_backing_store = 0;
      
      while (fgets(buffer, BUFFER_SIZE, input_fp) != NULL) {
        total_addresses++;
        int logical_address = atoi(buffer);

        // Calculate the page offset and logical page number from logical_address */
        int offset = logical_address & OFFSET_MASK;
        int logical_page = (logical_address >> OFFSET_BITS) & PAGE_MASK;
        ///////
    
        int physical_page = search_tlb(logical_page);
      
        // TLB hit
        if (physical_page != -1) {
          tlb_hits++;
         
        // TLB miss
        } else {
          physical_page = pagetable[logical_page];
          
          // Page fault
          if (physical_page == -1) {
          /* TODO */       
            page_faults++;
            
            if(free_page > 0){
              physical_page = FRAMES - free_page;
              free_page--;  
            }else{
              if(p){
                  int page = LRU();
                  physical_page =  pagetable[page];
                  pagetable[page] = -1;
              }else{
                  int page = FIFO();
                        physical_page =  pagetable[page];
                  pagetable[page] = -1;
              }
            } 
            fifo_next = (fifo_next + 1) % FRAMES;
            pagetable[logical_page] = physical_page;

            memcpy(main_memory + physical_page*PAGE_SIZE, backing + logical_page*PAGE_SIZE, PAGE_SIZE);
          }
          add_to_tlb(logical_page, physical_page);
        }
       
       //apply only if the replacement is LRU (need to set value to 0)
        if(p) {
	  //first, age value is incremented
	  LRU_node *curr = lru_head;

	  while(curr->next != NULL){
	    (curr->next)->age_val++;
	    curr = curr->next;
	  }

	  curr = lru_head;

	  int flag = 1;
	  while(curr->next != NULL){
	      curr = curr->next;
	      if(curr->data == logical_page){
		curr->age_val = 0;
		flag = 0;
		break;
	      }
	  }     
       
         if(flag){
		  //age value of page i is set to 0, after creating lru_node
		  LRU_node *new_node = malloc(sizeof(LRU_node));
		  new_node->data = logical_page;
		  new_node->age_val = 0;
		  new_node->next = NULL;

		  curr->next = new_node;
		  lru_size++;

		  int flag2 = 1;
		  if(FRAMES < lru_size){ //return LRU, same operation
		    LRU();
		  }
	  } 
        }
       
       
        int physical_address = (physical_page << OFFSET_BITS) | offset;
        signed char value = main_memory[physical_page * PAGE_SIZE + offset];
        
        printf("Virtual address: %d Physical address: %d Value: %d\n", logical_address, physical_address, value);
      }
      
      printf("Number of Translated Addresses = %d\n", total_addresses);
      printf("Page Faults = %d\n", page_faults);
      printf("Page Fault Rate = %.3f\n", page_faults / (1. * total_addresses));
      printf("TLB Hits = %d\n", tlb_hits);
      printf("TLB Hit Rate = %.3f\n", tlb_hits / (1. * total_addresses));
      
      return 0;
}
