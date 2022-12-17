#include "mmu.h"
#include "pagesim.h"
#include "swapops.h"
#include "stats.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/**
 * --------------------------------- PROBLEM 6 --------------------------------------
 * Checkout PDF section 7 for this problem
 * 
 * Page fault handler.
 * 
 * When the CPU encounters an invalid address mapping in a page table, it invokes the 
 * OS via this handler. Your job is to put a mapping in place so that the translation 
 * can succeed.
 * 
 * @param addr virtual address in the page that needs to be mapped into main memory.
 * 
 * HINTS:
 *      - You will need to use the global variable current_process when
 *      altering the frame table entry.
 *      - Use swap_exists() and swap_read() to update the data in the 
 *      frame as it is mapped in.
 * ----------------------------------------------------------------------------------
 */
void page_fault(vaddr_t addr) {
   // TODO: Get a new frame, then correctly update the page table and frame table
   vpn_t vpn = vaddr_vpn(addr);
   pte_t *pte = (pte_t *)(mem + (PTBR * PAGE_SIZE) + (vpn * sizeof(pte_t)));

   pte->pfn = free_frame();
   
   pte->valid = 1;

   if (swap_exists(pte)) {
      swap_read(pte, (void *)(mem + pte->pfn * PAGE_SIZE));
   } else {
      memset((uint8_t *)(mem + pte->pfn * PAGE_SIZE), 0, PAGE_SIZE);
   }

   frame_table[pte->pfn].mapped = 1;
   frame_table[pte->pfn].referenced = 0;
   frame_table[pte->pfn].process = current_process;
   frame_table[pte->pfn].vpn = vpn;
}

#pragma GCC diagnostic pop
