#include <stdlib.h>

#include "types.h"
#include "pagetable.h"
#include "global.h"
#include "process.h"

/*******************************************************************************
 * Finds a free physical frame. If none are available, uses a clock sweep
 * algorithm to find a used frame for eviction.
 *
 * @return The physical frame number of a free (or evictable) frame.
 */
pfn_t get_free_frame(void) {
   int i;

   /* See if there are any free frames */
   for (i = 0; i < CPU_NUM_FRAMES; i++)
      if (rlt[i].pcb == NULL)
         return i;

   /* FIX ME : Problem 5 */
   /* IMPLEMENT A CLOCK SWEEP ALGORITHM HERE */



   for(i = 0; i < CPU_NUM_PTE; i++) {
	
	// if there is invalid entry
   	if(!current_pagetable[i].valid) {

		return current_pagetable[i].pfn;
	}
   }
   		
   for(i = 0; i < CPU_NUM_PTE; i++) {

	// if there is not used=0 entry
   	if(!current_pagetable[i].used) {

		return current_pagetable[i].pfn;
	// if there is used=0 entry
	} else
		current_pagetable[i].used = 0;		
   }




   /* If all else fails, return a random frame */
   return rand() % CPU_NUM_FRAMES;
}
