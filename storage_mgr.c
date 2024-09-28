#include "storage_mgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

FILE *pageFilePointer;

// storage manager initalization function
extern void initStorageManager(void)
{
    pageFilePointer = NULL;
    printf(" ----- Storage Manager is initalized  -----\n");
}

// -----  PAGE FILE MANIPULATION BLOCKS ----- //

// This function creates a new page file with the given file name
extern RC createPageFile(char *fileName)
{
    pageFilePointer = fopen(fileName, "w+"); // creates a new file or overwrites an existing file
    char overwrite = '\0';

    if (!pageFilePointer)
    {
        RC_message = "File creation failed";
        return RC_ERROR;
    }

    // allocating memory for an empty page
    SM_PageHandle emptyPointer = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));

    if (fwrite(emptyPointer, sizeof(char), PAGE_SIZE, pageFilePointer) < PAGE_SIZE) // writes the contents of the empty page to the file
    {
        return RC_WRITE_FAILED;
    }
    fseek(pageFilePointer, 0, SEEK_END);
    fclose(pageFilePointer);
    free(emptyPointer);

    RC_message = "A new file is created succesfully";
    return RC_OK;
}

// This function opens an existing page file with the given file name
extern RC openPageFile(char *fileName, SM_FileHandle *fHandle)
{

    pageFilePointer = fopen(fileName, "r+"); // allows reading and writing to the file

    if (pageFilePointer == NULL) // checks if the file is present or not.
    {
        return RC_FILE_NOT_FOUND;
    }

    if (fseek(pageFilePointer, 0, SEEK_END) != 0 && ftell(pageFilePointer) == -1)
    {
        fclose(pageFilePointer);
        return RC_ERROR;
    }
    
    int fileSize = ftell(pageFilePointer); // ftell returns the current position of the file position indicator
    int totnp;
    if (fileSize % PAGE_SIZE == 0)
    {
        totnp = (fileSize / PAGE_SIZE);
    }
    else
    {
        totnp = (fileSize / PAGE_SIZE + 1);
    }

    fHandle->fileName = fileName;
    fHandle->curPagePos = 0;
    fHandle->mgmtInfo = pageFilePointer;
    fHandle->totalNumPages = totnp;
    fclose(pageFilePointer);
    return RC_OK;
}

// This function closes the file
extern RC closePageFile(SM_FileHandle *fHandle)
{
    if(pageFilePointer != NULL){
        pageFilePointer = NULL;
    }

  RC_message = "File closed successfully";
  return RC_OK;
}

// This function deletes the page file with the given file name
extern RC destroyPageFile(char *fileName)
{
    if (remove(fileName) == -1) // remove() returns -1 if there is an error while deleting the file
    {
        RC_message = "Error while destroying file";
        return RC_ERROR;
    }
    RC_message = "File destroyed successfully";
    return RC_OK;
}

// -----  READ BLOCKS ----- //

// This function reads a block from a disk file and stores it in a memory page.
RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{

    if (pageNum > fHandle->totalNumPages || pageNum < 0)
    {
        return RC_READ_NON_EXISTING_PAGE;
    }
    else
    {
        long fset = pageNum * PAGE_SIZE;

        pageFilePointer = fopen(fHandle->fileName, "r");

        if (pageFilePointer != NULL)
        {

            fseek(pageFilePointer, fset, SEEK_SET);
            fread(memPage, sizeof(char), PAGE_SIZE, pageFilePointer);

            // setting current page to the given pageNum
            fHandle->curPagePos = pageNum;

            fclose(pageFilePointer);
        }

        return RC_OK;
    }
}

// This function returns the current page position in the file
extern int getBlockPos(SM_FileHandle *fHandle)
{
    int position = -1;
    bool getBlock = false;
    if (getBlock == false)
    {
        position = fHandle->curPagePos;
    }
    return position;
}

// This function reads first block from a disk file with pageNum = 0 and stores it in a memory page.
extern RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    return readBlock(0, fHandle, memPage);
}

// This function reads the previous block from a disk file and stores it in a memory page.
extern RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    int prev_pos = (fHandle->curPagePos - 1);
    return readBlock(prev_pos, fHandle, memPage);
}

// This function reads the current block from a disk file and stores it in a memory page.
extern RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    int current_pos = (fHandle->curPagePos);
    return readBlock(current_pos, fHandle, memPage);
}

// This function reads the next block from a disk file and stores it in a memory page.
extern RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    int next_pos = (fHandle->curPagePos + 1);
    return readBlock(next_pos, fHandle, memPage);
}

// This function reads the last block from a disk file and stores it in a memory page.
extern RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    int last_pos = (fHandle->totalNumPages - 1);
    return readBlock(last_pos, fHandle, memPage);
}


// -----  WRITE BLOCKS ----- //

// This function writes a given data (memPage) to the specified block (pageNum) in the file
RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    if (ensureCapacity(pageNum, fHandle) == RC_OK)
    {
        FILE *pageFilePointer;
        if ((FILE *)fHandle->mgmtInfo == NULL)
        {
            return RC_FILE_NOT_FOUND;
        }

        pageFilePointer = fopen(fHandle->fileName, "rb+");

        if (fseek(pageFilePointer, (pageNum * PAGE_SIZE), SEEK_SET) != 0)
        {
            return RC_READ_NON_EXISTING_PAGE;
        }

        else if (fwrite(memPage, sizeof(char), PAGE_SIZE, pageFilePointer) != PAGE_SIZE)
        {
            return RC_WRITE_FAILED;
        }
        fHandle->curPagePos = pageNum;

        fclose(pageFilePointer);
        return RC_OK;
    }
}


// This function writes the memory page "memPage" to the current block position in the file
extern RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    int currentBlock = getBlockPos(fHandle);
    return writeBlock(currentBlock, fHandle, memPage);
}

// This function appends an empty block (a block of zeros) to the end of the file
extern RC appendEmptyBlock(SM_FileHandle *fHandle)
{
    // Moving the pointer to the end of the file
    if (fseek(pageFilePointer, 0, SEEK_END) == 0)
    {
        SM_PageHandle emptyPointer = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
        int emptyWriteSize = fwrite(emptyPointer, sizeof(char), PAGE_SIZE, pageFilePointer); // allocates an empty block of memory

        if (emptyWriteSize == PAGE_SIZE)
        {
            fHandle->totalNumPages++; // updates the value to reflect the addition of the new block
        }
        free(emptyPointer);
    }
    else
    {
        return RC_WRITE_FAILED;
    }
    return RC_OK;
}

// This function checks if a file has enough capacity to store a specified number of pages
extern RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle)
{
    if (numberOfPages <= fHandle->totalNumPages) // checks if there is enough capacity
    {
        RC_message = "There is enough capacity";
        return RC_OK;
    }
    if (numberOfPages > fHandle->totalNumPages)
    {
        int extra_no_of_pages = numberOfPages - fHandle->totalNumPages;
        for (int i = 0; i < extra_no_of_pages; i++)
        {
            appendEmptyBlock(fHandle);
        }
    }
    return RC_OK;
}
