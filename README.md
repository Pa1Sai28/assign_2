# PROGRAMMING ASSIGNMENT 2 - BUFFER MANAGER


## Group: 17
NAGA PAVANSAI KUMAR VARIKUTI (A20542909)
SAI SIVANI DUKKIPATI (A20547981)
PRANITHA NADIMPALLI (A20541099)


# FILES INCLUDED IN THE ASSIGNMENT


- README.md
- buffer_mgr_stat.c
- buffer_mgr_stat.h
- buffer_mgr.c
- buffer_mgr.h
- dberror.c
- dberror.h
- dt.h
- Makefile
- storage_mgr.c
- storage_mgr.h
- test_assign2_1.c
- test_helper.h


# HOW TO RUN THE ASSIGNMENT


Step 1: Open the terminal, go to project root (assign1).
           cd <project-path>

Step 2: Run the command "make clean" to cleanup the old compiled files.

Step 3: Run the command "make" to compile all the project files.

Step 4: Then, run the command "make run" to execute the output of the project.

Step 5: Finally, run the command "make clean" to clean up the executable files created for the previous instance and repeat steps 3 and 4 to re-run the code.

step 6: When wanted to run the test_assign2_2.c , edit the code in make file  where in SRC test_assign2_1.c with test_assign2_2.c and OFILS with test_assign2_1.o with test_assign2_2.o 

Step 7: repeat the steps 2 , 3 , 4 in order to get the output for the test_assign2 testing document.

(you can see the commented file in the makefile)

## BUFFER POOL FUNCTIONS

1. initBufferPool : 
* This function initializes a buffer pool by allocating memory for page frames, setting default values for the frame properties, and initializing the replacement strategy.
* The function takes two inputs - a pointer to the buffer pool, a page file, number of pages in the buffer pool, the replacement strategy to be used, and stratData.
* If the replacement strategy is not PAGE_LRU or PAGE_LFU, it sets the lru_hit and lfu_ref fields to 0 for each page frame in the array.
* The function sets the mgmtData field in the buffer pool structure to point to the page frame array.
* The function returns successful.
* Overall, the function prepares the buffer pool for use by setting up the page frames with default values and initializing the replacement strategy.

2. shutdownBufferPool: 
* This function is responsible for shutting down the buffer pool and freeing up any resources that were allocated for the pool. 
* The function takes one input - a pointer to the buffer pool.
* The function calls forceFlushPool() to write any dirty pages to disk before shutting down the buffer pool.
* The function then checks the fix count for each page frame in the buffer pool. If any page has a non-zero fix count, it sets result to true.
* If all pages have a fix count of zero, result remains false.
* The function sets the mgmtData field in the buffer pool structure to NULL to indicate that the buffer pool has been shut down.
* Finally, the function checks whether there were any pinned pages in the buffer pool. 

3. forceFlushPool: 
* This function is responsible for writing any dirty pages from the buffer pool to disk. 
* The function takes one input - a pointer to the buffer pool.
* The function loops through each page frame in the buffer pool.
* For each page frame, the function checks whether the dirty_bit_flag is set to 1 and the fix_count is 0. If so, the page frame is dirty and can be written to disk.
* The function opens the page file using openPageFile() and obtains a handle to the file.
* The function obtains a pointer to the page data from the data field of the page frame and the page number from the page_no field of the page frame.
* The function writes the page data to disk using writeBlock().
* The function sets the dirty_bit_flag to 0 to indicate that the page is no longer dirty.
* The function increments the write_count variable to keep track of the number of pages written to disk.
* After all dirty pages have been written to disk, the function returns successfully.
* Overall, the forceFlushPool() function ensures that any dirty pages in the buffer pool are written to disk before the buffer pool is shut down or the data in the pool is lost.

## PAGE MANAGEMENT FUNCTIONS

4. pinPage:
* This function reads a page from the disk and stores its content in a page frame within the buffer pool.
* If the page is already in memory, then its page frame is directly accessed, and its content is returned.
* If the buffer pool is full, then the page replacement strategy is applied to evict an existing page frame and replace it with the new page frame.
* FIFO, LRU, CLOCK, LFU are the replacement strategies we use for pinning a page. 


5. markDirty: 
* This function is used to mark the page dirty indicating that the page has been modified by a client.
* It searches for the page with the given page number in the buffer pool, it marks a page as dirty by setting its dirty_bit_flag to 1, if found.
* If the page is marked as dirty, it returns RC_OK. Otherwise, it returns RC_ERROR.

6. unpinPage: 
* This function is used to unpin a page if the client no longer needs that page.
* It iterates through the loop to find the page with a matching page number to the one in the given page handle.
* If the page is found, its fix count is decremented and the function returns RC_OK. If the page is not found, the function returns RC_ERROR.

7. Force page: 
* This function is used to write the current page's content back to page file on disk.
* It uses a pointer to a structure PFrames to access the buffer pool's memory data and searches for a matching page number to write the page data to disk.
* Once a matching page is found, it writes the data to disk and sets the dirty_bi_flag to zero to indicate that the page is no longer modified.
 
## STATISTICAL FUNCTIONS 

8. getFrameContents: 
* This function takes a buffer pool pointer as input and returns an array of page numbers corresponding to the pages currently present in the buffer pool frames.

9. getDirtyFlags: 
* This function takes a buffer pool pointer as input and returns an array of boolean values indicating whether the pages present in the buffer pool frames have been modified (dirty) or not.

10. getFixCounts: 
* This function takes a buffer pool pointer bm as input and returns an array of integers indicating the number of times each page in the buffer pool frames has been pinned (fixed).

11. getNumReadIO: 
* This function takes a buffer pool pointer bm as input and returns the number of pages that have been read from the disk into the buffer pool.

12. getNumWriteIO: 
* This function takes a buffer pool pointer bm as input and returns the number of pages that have been written from the buffer pool to the disk.

## PAGE REPLACEMENT STRATEGIES

13. FIFO :
* This function implements the First-In-First-Out (FIFO) page replacement strategy for a buffer pool in a database management system.
* The function takes two inputs - a pointer to the buffer pool and a pointer to a page frame structure, which contains information about the page to be added to the buffer pool.
* The function first retrieves the buffer pool's management data (i.e., the array of page frames) using the input buffer pool pointer.
* It then calculates the index of the page frame to be replaced. 
* The function then iterates through the page frames in the buffer pool to find an empty page frame or a page frame with a fix count of zero.
* If a suitable page frame is found, the function checks whether it is dirty. 
* If the page is dirty, the function writes it back to disk using the writeBlock() function.
* The function then updates the page frame's data, page number, dirty bit flag, and fix count with the information from the input page frame structure.
* Finally, the function breaks out of the loop and returns.
* If no suitable page frame is found in the loop, the function simply increments the index variable and continues searching until it finds an empty or non-fixed page frame to replace.

14. LRU :
* This function implements the Least Recently Used (LRU) page replacement strategy for a buffer pool in a database management system.
* The function takes two inputs - a buffer pool and a page, and replaces the least recently used page in the buffer pool with the new page. 
* The function first iterates through the buffer pool to find the page with the smallest hit value, updating page index and hit count accordingly.
* If the least recently used page is dirty, the function then writes it back to disk and increment the write count.
* The function then replaces the least recently used page with the new page, updating its hit count, dirty bit flag, fix count, and page number.
* The function ensures that pages with fix count > 0 are not replaced, as they are currently being used by one or more clients. The LRU algorithm chooses to evict the page that has not been accessed for the longest period of time

15. CLOCK :
* This function implements the Clock page replacement strategy for a buffer pool in a database management system.
* The function takes two inputs - a buffer pool and a page frame.
* The function first casts the management data of the buffer pool to an array of PFrames to access the page frames in the pool.
* Function then enters a loop that continues indefinitely until it finds an available page frame to write the new page into.
* If the current page frame pointed to by the clock pointer has a non-zero LRU hit count, it updates the LRU hit count to zero and increments the clock pointer to move to the next frame.
* If the current page frame pointed to by the clock pointer has a zero LRU hit count, the function checks if the dirty bit flag is set to 1, indicating that the page has been modified and needs to be written back to disk.
* If the dirty bit flag is set to 1, the function opens the page file and writes the page back to disk using the writeBlock() function.
* Function then replaces the current page in the page frame pointed to by the clock pointer with the new page passed as input to the function.
* Finally, The function updates the clock pointer to move to the next frame and exits the loop.

16. LFU:
* This function implements the LFU (Least Frequently Used) page replacement strategy for a buffer pool in a database management system.
* The function takes two inputs - a buffer pool and a page frame.
* The function retrieves the page frames from the buffer pool and initializes some variables.
* The function finds the least frequently used page frame by looping through the page frames and checking their fix count. The page frame with the least fix count is selected as the victim page frame.
* If the victim page frame is dirty (i.e., it has been modified), its contents are written back to the page file.
* The data, dirty bit flag, fix count, and page number of the new page frame are then stored in the victim page frame.
* The function updates the pointer to the next page frame to be replaced.
* Overall, the function selects the page frame that has been referenced the least number of times for replacement.

