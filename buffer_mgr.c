#include <stdio.h>
#include <stdlib.h>
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include <math.h>

int buffer_size = 0;
bool isTrue = true;
int write_count = 0;
int r_index = 0;

typedef struct Page
{
	SM_PageHandle data;
	PageNumber page_no;
	int fix_count;
	int lru_hit;
	int lfu_ref;
	int dirty_bit_flag;
} PFrames;

int f_count = 0, rs_lfu_pointer = 0;
int rs_clock_pointer = 0;

bool allFlags = true;
int value_zero = 0, value_one = 1;

/*
Implementing the FIFO (First In, First Out) page replacement algorithm for a buffer pool,
where a new page frame is added to the buffer pool and the oldest unpinned page frame is replaced.
*/
void FIFO(BM_BufferPool *const bm, PFrames *page)
{
	PFrames *pframe = (PFrames *)bm->mgmtData;
	int indx = r_index % buffer_size;

	SM_FileHandle file_handle;

	bool allFlags = true;

	for (int i = 0; i < buffer_size; i++)
	{
		if (pframe[indx].fix_count == value_zero)
		{
			if (pframe[indx].dirty_bit_flag == value_one)
			{

				openPageFile(bm->pageFile, &file_handle);

				if (value_one)
				{
					int pNo = pframe[indx].page_no;
					writeBlock(pNo, &file_handle, pframe[indx].data);
				}

				++write_count;
			}

			pframe[indx].data = page->data;

			int new_page_no = page->page_no;
			pframe[indx].page_no = new_page_no;

			int new_dirty_bit_flag = page->dirty_bit_flag;
			pframe[indx].dirty_bit_flag = new_dirty_bit_flag;

			int new_fix_count = page->fix_count;
			pframe[indx].fix_count = new_fix_count;
			break;
		}
		else
		{
			indx = indx + 1;
			if (indx % buffer_size == 0)
			{
				indx = 0;
			}
		}
	}
}

/*
Implementing the LRU (Least Recently Used) page replacement strategy in a buffer manager.
It selects the page frame that has the lowest LRU hit count and replaces it with the new page.
*/
void LRU(BM_BufferPool *const bm, PFrames *page)
{
	int lHR, lHC, indx = 0, bs = buffer_size;

	PFrames *pframe = (PFrames *)bm->mgmtData;
	SM_FileHandle file_handle;

	while (indx < bs)
	{
		if (pframe[indx].fix_count == value_zero)
		{
			lHC = pframe[indx].lru_hit;
			lHR = indx;
			break;
		}
		++indx;
	}

	indx = lHR + 1;

	while (indx < bs)
	{
		if (pframe[indx].lru_hit < lHC)
		{
			lHC = pframe[indx].lru_hit;
			lHR = indx;
		}
		++indx;
	}

	if (pframe[lHR].dirty_bit_flag == value_one)
	{

		openPageFile(bm->pageFile, &file_handle);

		if (value_one)
		{
			int pNo = pframe[lHR].page_no;
			SM_PageHandle mem_p = pframe[lHR].data;
			writeBlock(pNo, &file_handle, mem_p);
		}

		++write_count;
	}

	pframe[lHR].data = page->data;
	pframe[lHR].lru_hit = page->lru_hit;

	int new_dirty_bit_flag = page->dirty_bit_flag;
	pframe[lHR].dirty_bit_flag = new_dirty_bit_flag;

	int new_fix_count = page->fix_count;
	pframe[lHR].fix_count = new_fix_count;

	int new_page_no = page->page_no;
	pframe[lHR].page_no = new_page_no;
}

/*
Implementing the CLOCK page replacement algorithm which maintains a circular buffer of page frames
and a clock hand that points to the next frame to be evicted.
*/
void CLOCK(BM_BufferPool *const bm, PFrames *page)
{
	PFrames *pframe = (PFrames *)bm->mgmtData;
	SM_FileHandle file_handle;

	while (value_one)
	{
		int value = rs_clock_pointer % buffer_size;
		if (value == 0)
		{
			rs_clock_pointer = value_zero;
		}

		if (pframe[rs_clock_pointer].lru_hit != value_zero)
		{
			pframe[rs_clock_pointer++].lru_hit = value_zero;
		}
		else
		{
			if (pframe[rs_clock_pointer].dirty_bit_flag == 1)
			{

				openPageFile(bm->pageFile, &file_handle);

				if (value_one)
				{
					int pNo = pframe[rs_clock_pointer].page_no;
					SM_PageHandle mem_p = pframe[rs_clock_pointer].data;
					writeBlock(pNo, &file_handle, mem_p);
				}

				++write_count;
			}

			pframe[rs_clock_pointer].data = page->data;
			pframe[rs_clock_pointer].lru_hit = page->lru_hit;

			int new_page_no = page->page_no;
			pframe[rs_clock_pointer].page_no = new_page_no;

			int new_dirty_bit_flag = page->dirty_bit_flag;
			pframe[rs_clock_pointer].dirty_bit_flag = new_dirty_bit_flag;

			int new_fix_count = page->fix_count;
			pframe[rs_clock_pointer].fix_count = new_fix_count;

			++rs_clock_pointer;

			break;
		}
	}
}

/*
Implementing the LFU (Least Frequently Used) page replacement algorithm for a buffer pool,
where the page frame with the least frequency of reference (i.e., the least commonly used page frame)
is replaced with a new page frame.
*/
void LFU(BM_BufferPool *const bm, PFrames *page)
{
	PFrames *pframe = (PFrames *)bm->mgmtData;
	int least_freq_ref, least_freq_index = rs_lfu_pointer, bs = buffer_size;
	SM_FileHandle file_handle;

	int i = 0, j = 0;

	while (i < bs)
	{
		if (pframe[least_freq_index].fix_count == value_zero)
		{
			least_freq_index = (least_freq_index + i) % buffer_size;
			least_freq_ref = pframe[least_freq_index].lfu_ref;
			break;
		}
		++i;
	}

	i = (least_freq_index + value_one);
	i = i % buffer_size;

	while (j < bs)
	{
		if (pframe[i].lfu_ref < least_freq_ref)
		{
			least_freq_index = i;
			least_freq_ref = pframe[i].lfu_ref;
		}
		i = (i + 1) % buffer_size;
		++j;
	}

	if (pframe[least_freq_index].dirty_bit_flag == 1)
	{

		openPageFile(bm->pageFile, &file_handle);

		if (value_one)
		{
			int pNo = pframe[least_freq_index].page_no;
			SM_PageHandle mem_p = pframe[least_freq_index].data;
			writeBlock(pNo, &file_handle, mem_p);
		}

		++write_count;
	}

	pframe[least_freq_index].data = page->data;

	int new_dirty_bit_flag = page->dirty_bit_flag;
	pframe[least_freq_index].dirty_bit_flag = new_dirty_bit_flag;

	int new_fix_count = page->fix_count;
	pframe[least_freq_index].fix_count = new_fix_count;

	int new_page_no = page->page_no;
	pframe[least_freq_index].page_no = new_page_no;

	rs_lfu_pointer = least_freq_index + 1;
}

// -----  BUFFER POOL FUNCTIONS ----- //

/*
"initBufferPool" function initializes a buffer pool with a specified number of pages,
replacement strategy, and page file name.
*/
RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, const int numPages, ReplacementStrategy strategy, void *stratData)
{
	int value_neg = -1, indx = 0;
	buffer_size = numPages;

	bm->strategy = strategy;

	PFrames *pframe = malloc(numPages * sizeof(PFrames));

	bm->pageFile = (char *)pageFileName;
	bm->numPages = numPages;

	while (indx < buffer_size)
	{

		pframe[indx].page_no = value_neg;
		pframe[indx].dirty_bit_flag = value_zero;

		if (!value_zero)
		{
			pframe[indx].lru_hit = value_zero;
			pframe[indx].lfu_ref = value_zero;
		}
		isTrue = true;
		pframe[indx].fix_count = value_zero;
		pframe[indx].data = NULL;

		++indx;
	}

	write_count = rs_lfu_pointer = rs_clock_pointer = value_zero;
	bm->mgmtData = pframe;

	return RC_OK;
}

/*
"forceFlushPool" function iterates over all the pages in the buffer pool and
writes the dirty pages back (fix count=0) to disk if they are not pinned.
*/
RC forceFlushPool(BM_BufferPool *const bm)
{
	PFrames *pframe = (PFrames *)bm->mgmtData;

	int indx = 0;

	while (indx < buffer_size)
	{

		if (pframe[indx].dirty_bit_flag == value_one && pframe[indx].fix_count == value_zero)
		{
			SM_FileHandle file_handle;
			openPageFile(bm->pageFile, &file_handle);

			SM_PageHandle mem_p = pframe[indx].data;
			int pNo = pframe[indx].page_no;
			writeBlock(pNo, &file_handle, mem_p);

			pframe[indx].dirty_bit_flag = value_zero;
			write_count++;
		}
		++indx;
	}
	return RC_OK;
}

/*
"shutdownBufferPool" function shutdowns a buffer pool, ensuring that any changes made to the pages
in the buffer pool are written to disk and all the resources are freed.
*/
RC shutdownBufferPool(BM_BufferPool *const bm)
{
	bool result = false;
	int i = 0, condition = 0;

	PFrames *pframe = (PFrames *)bm->mgmtData;

	forceFlushPool(bm);

	for (i = 0; i < buffer_size; ++i)
	{
		condition = pframe[i].fix_count;
		if (condition != 0)
		{
			result = true;
		}
		else
		{
			result = false;
		}
	}

	bm->mgmtData = NULL;
	condition = -1;
	free(pframe);

	if (condition == -1 && result)
	{
		return RC_PINNED_PAGES_IN_BUFFER;
	}
	return RC_OK;
}

// -----  PAGE MANAGEMENT FUNCTIONS ----- //

/*
"markDirty" function marks a page as dirty in the buffer pool
*/
RC markDirty(BM_BufferPool *const bm, BM_PageHandle *const page)
{
	int i = 0;
	bool checker = false;
	PFrames *dirty_pointer = (PFrames *)bm->mgmtData;
	while (i < buffer_size && !checker)
	{
		if (dirty_pointer[i].page_no == page->pageNum)
		{
			dirty_pointer[i].dirty_bit_flag = value_one;
			allFlags = true;
			checker = true;
			break;
		}
		i++;
	}
	if (allFlags && checker)
	{
		return RC_OK;
	}
	return RC_ERROR;
}

/*
"unpinPage" function decreases the fix count of a page in the buffer pool.
*/
RC unpinPage(BM_BufferPool *const bm, BM_PageHandle *const page)
{
	PFrames *unpin_pointer = (PFrames *)bm->mgmtData;
	bool pageFound = false;
	int i = 0;
	while (i < buffer_size)
	{ 
		if (unpin_pointer[i].page_no == page->pageNum)
		{
			unpin_pointer[i].fix_count--;
			pageFound = true;
			isTrue = true;
			break;
		}
		i++;
	}
	if (!pageFound)
	{
		return RC_ERROR;
	}
	return RC_OK;
}

/*
"pinPage" function reads a page from the disk and stores its content in a page frame within the buffer pool.
If the page is already in memory, then its page frame is directly accessed, and its content is returned.
If the buffer pool is full, then the page replacement strategy is applied to evict an existing page frame and replace it with the new page frame.
*/
RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, 
	    const PageNumber pageNum)
{
	bool check= true, flag=false;
	int indx=0;
	PFrames *pframe = (PFrames *)bm->mgmtData;
	
	while(check){
		break;
	}
	SM_FileHandle file_handle;
	if(pframe[0].page_no == -1)
	{
		openPageFile(bm->pageFile, &file_handle);
		pframe[0].data = (SM_PageHandle) malloc(PAGE_SIZE);
		flag=true;
		ensureCapacity(pageNum,&file_handle);
		check=false;
		readBlock(pageNum, &file_handle, pframe[0].data);
		pframe[0].page_no = pageNum;
		indx=indx+value_one-value_zero;
		pframe[0].fix_count++;
		r_index = value_one-value_one;
		f_count = value_zero;
		pframe[0].lru_hit = f_count;	
		pframe[0].lfu_ref = 0;
		indx=indx+value_one-value_zero-1;
		page->pageNum = pageNum;
		page->data = pframe[0].data;
		
		return RC_OK;		
	}
	else
	{	
		int i;
		indx=0;
		bool isBufferFull = true;
		while(!check){
			break;
		}
		for(i = 0; i < buffer_size; i++)
		{
			indx+=10;
			if(pframe[i].page_no != -1)
			{	
				indx-=5;
				if(pframe[i].page_no == pageNum)
				{
					pframe[i].fix_count++;
					indx--;
					isBufferFull = false;
					f_count+=1; 
					if(bm->strategy == RS_LRU){
						pframe[i].lru_hit = f_count;
						while(check){
							break;
						}
					} else if(bm->strategy == RS_CLOCK){
						pframe[i].lru_hit = 1;
						flag=true;
					}else if(bm->strategy == RS_LFU)
						pframe[i].lfu_ref++;
						flag=false;
					
					page->pageNum = pageNum;
					indx++;
					page->data = pframe[i].data;

					rs_clock_pointer++;
					break;
				}				
			} else {
				flag=false;
				openPageFile(bm->pageFile, &file_handle);
				pframe[i].data = (SM_PageHandle) malloc(PAGE_SIZE);
				if(flag){}
				readBlock(pageNum, &file_handle, pframe[i].data);
				pframe[i].page_no = pageNum;
				r_index=r_index+1;	
				pframe[i].fix_count = 1;
				f_count=f_count+1;
				pframe[i].lfu_ref = 0;
				
				if(bm->strategy == RS_LRU){
					pframe[i].lru_hit = f_count;
					check=flag=true;				
				}else if(bm->strategy == RS_CLOCK)
					flag=true;
					pframe[i].lru_hit = 1;
						
				page->pageNum = pageNum;
				if(check){}
				page->data = pframe[i].data;
				
				isBufferFull = false;
				break;
			}
		}
		
		if(isBufferFull == true)
		{
			if(flag){}
			PFrames *newPage = (PFrames *) malloc(sizeof(PFrames));		
			
			openPageFile(bm->pageFile, &file_handle);
			newPage->data = (SM_PageHandle) malloc(PAGE_SIZE);
			check=1;
			readBlock(pageNum, &file_handle, newPage->data);
			newPage->page_no = pageNum;
			r_index=r_index+1;
			newPage->dirty_bit_flag = 0;
			f_count=f_count+1;
			newPage->fix_count = 1;
			newPage->lfu_ref = 0;
			
			if(bm->strategy == RS_LRU){
				newPage->lru_hit = f_count;
				while(check){
					break;
				}				
			}else if(bm->strategy == RS_CLOCK)
				newPage->lru_hit = 1;

			page->pageNum = pageNum;
			indx++;
			page->data = newPage->data;			

			if (bm->strategy == RS_FIFO) {
				FIFO(bm, newPage);
			} else if (bm->strategy == RS_LRU) {
				LRU(bm, newPage);
			} else if (bm->strategy == RS_CLOCK) {
				CLOCK(bm, newPage);
			} else if (bm->strategy == RS_LFU) {
				LFU(bm, newPage);
			} else if (bm->strategy == RS_LRU_K) {
				printf("\n LRU-K replacement strategy is not implemented");
			} else {
				printf("\nReplacement strategy is not implemented\n");
			}
						
		}		
		return RC_OK;
	}	
}

/*
"forcePage" function writes a page in the buffer pool back to disk if it is dirty.
*/
RC forcePage(BM_BufferPool *const bm, BM_PageHandle *const page)
{
	int i = 0;

	PFrames *force_pointer = (PFrames *)bm->mgmtData;
	bool checkerPage = false;
	SM_FileHandle file_handle;

	while (i < buffer_size)
	{
		int bm_page = page->pageNum;

		SM_PageHandle mem_page = force_pointer[i].data;
		int disk_page = force_pointer[i].page_no;
		if (disk_page == bm_page)
		{
			openPageFile(bm->pageFile, &file_handle);
			allFlags = true;
			writeBlock(disk_page, &file_handle, mem_page);
			write_count++;

			force_pointer[i].dirty_bit_flag = value_zero;
		}
		i++;
	}
	return RC_OK;
}

// -----  STATISTICAL FUNCTIONS ----- //

/*
"getNumReadIO" function returns the number of pages that have been read from disk since the buffer pool bm was initialized.
*/
int getNumReadIO(BM_BufferPool *const bm)
{
	int readIOPages = 0;
	readIOPages = r_index + 1;
	return readIOPages;
}

/*
"getFrameContents" function returns an array of page numbers for all the pages that are currently held in the buffer pool frames
*/
PageNumber *getFrameContents(BM_BufferPool *const bm)
{

	int conFlag = -1;
	PageNumber *getContentsArray = malloc(buffer_size * sizeof(PageNumber));
	int indxi = 0;
	PFrames *pframe = (PFrames *)bm->mgmtData;

	while (indxi < buffer_size)
	{
		if (indxi > 0)
		{
		}
		if (pframe[indxi].page_no == conFlag && conFlag == -1)
		{
			getContentsArray[indxi] = NO_PAGE;
		}
		else
		{
			getContentsArray[indxi] = pframe[indxi].page_no;
		}
		indxi++;
	}
	return getContentsArray;
}

/*
"getDirtyFlags" function in this implementation returns an array of boolean flags indicating whether the pages in the buffer pool are "dirty"
(i.e. have been modified since being loaded into the buffer pool) or not.
*/
bool *getDirtyFlags(BM_BufferPool *const bm)
{

	bool isCrrt = true;
	bool *dirFlag = malloc(buffer_size * sizeof(bool));

	bool isInCrrt = false;
	PFrames *dirtyFlagsFrame = (PFrames *)bm->mgmtData;

	int getDirtFlag = 1, idxi = 0;
	for (idxi = 0; idxi < buffer_size; idxi++)
	{
		if (getDirtFlag)
		{
		}
		if (isCrrt == true && dirtyFlagsFrame[idxi].dirty_bit_flag != getDirtFlag)
		{
			dirFlag[idxi] = isInCrrt;
		}
		else
		{
			dirFlag[idxi] = isCrrt;
		}
		getDirtFlag = 1;
	}

	return dirFlag;
}

/*
"getNumWriteIO" function returns the number of pages written to disk so far by the buffer pool manager associated with the given buffer pool.
*/
int getNumWriteIO(BM_BufferPool *const bm)
{
	int writeIOPages;
	writeIOPages = write_count;
	return writeIOPages;
}

/*
"getfix_counts" function returns an array of integers, where each element of the array represents the
number of clients currently holding a fixed reference to the corresponding page frame in the buffer pool.
*/
int *getFixCounts(BM_BufferPool *const bm)
{
	PFrames *pframe = (PFrames *)bm->mgmtData;
	int fix_countFlag = -1;

	int *fix_counts = (int *)malloc(sizeof(int) * buffer_size);

	bool isFix = true;
	int idxi;
	for (idxi = 0; idxi < buffer_size; idxi++)
	{

		if (isFix == true && pframe[idxi].fix_count != fix_countFlag)
		{
			fix_counts[idxi] = pframe[idxi].fix_count;
			isFix = true;
		}
		else
		{
			isFix = true;
			fix_counts[idxi] = 0;
		}
		fix_countFlag = -1;
	}

	return fix_counts;
}
