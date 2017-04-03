#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
//#include <sys/types.h>
#include <string.h>

#include "storage_mgr.h"


//#define RC_ERROR -1
#define RC_VALID 31415926


int isValid(SM_FileHandle *fHandle) {
    if ((int)fHandle->mgmtInfo == RC_VALID) {
        return 1;
    }
    return 0;
}

int getFilePageNumber(FILE *fp) {
    struct stat buf;
    fstat(fileno(fp), &buf);
    return buf.st_size / PAGE_SIZE;
}

int getLastPageNumber(char *fileName) {
    FILE *fp = fopen(fileName, "r");
    int result = getFilePageNumber(fp);
    fclose(fp);
    return result;
}


void initStorageManager(void) {
    printf("Storage Manager Initialized\n");
}

RC createPageFile(char *fileName) {
    FILE *fp;
    SM_FileHandle fh;
    RC r;
    fp = fopen(fileName, "r");
    int seekFlag;
    int writeFlag;
    /* Check status of file */
    if (fp != NULL) {
        fclose(fp);
        return RC_ERROR;
    }
    fp = fopen(fileName, "w");
    /* Allocates memory and intializes to Zero */
    SM_PageHandle memPage = (SM_PageHandle)malloc(PAGE_SIZE);
    memset(memPage, 0, PAGE_SIZE);
    seekFlag=fseek(fp, 0, SEEK_SET);
    if ( seekFlag!= 0) {
        return RC_WRITE_FAILED;
    }
    writeFlag=fwrite(memPage, 1, PAGE_SIZE, fp);
    if (writeFlag != PAGE_SIZE) {
        return RC_WRITE_FAILED;
    }
    fclose(fp);
    return RC_OK;
}

RC openPageFile(char *fileName, SM_FileHandle *fHandle) {
    if (isValid(fHandle)) {
        return RC_OK;
    }

    FILE *fp;

    fp = fopen(fileName, "r");
    if (fp == NULL) {
        return RC_FILE_NOT_FOUND;
    }
    fHandle->fileName = fileName;
    fHandle->totalNumPages = getFilePageNumber(fp);
    fHandle->curPagePos = 0;
    fHandle->mgmtInfo = (void *)RC_VALID;
    fclose(fp);
    return RC_OK;
}

RC closePageFile(SM_FileHandle *fHandle) {
	fHandle->mgmtInfo = NULL;
    	return RC_OK;
    } 

RC destroyPageFile(char *fileName) {
    FILE *fp;

    fp = fopen(fileName, "r");
    if (fp == NULL) {
     return RC_ERROR;
    }
    fclose(fp);
   
    remove(fileName);
    return RC_OK;
}


RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
    FILE *fp;

    if (!isValid(fHandle)) {
        return RC_ERROR;
    }

    fp = fopen(fHandle->fileName, "r");
    if (pageNum >= fHandle->totalNumPages || pageNum < 0) {
        fclose(fp);
        return RC_READ_NON_EXISTING_PAGE;
    }
    fseek(fp, pageNum * PAGE_SIZE, SEEK_SET);
    fread(memPage, 1, PAGE_SIZE, fp);
    fHandle->curPagePos = pageNum;
    fclose(fp);
    return RC_OK;
}

int getBlockPos(SM_FileHandle *fHandle) {
    return fHandle->curPagePos;
}

RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    return readBlock(0, fHandle, memPage);
}

RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    return readBlock(fHandle->curPagePos - 1, fHandle, memPage);
}

RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    return readBlock(fHandle->curPagePos, fHandle, memPage);
}

RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    return readBlock(fHandle->curPagePos + 1, fHandle, memPage);
}

RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    return readBlock(fHandle->totalNumPages - 1, fHandle, memPage);
}


RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
    FILE *fp;

    if (!isValid(fHandle)) {
        return RC_ERROR;
    }

    fp = fopen(fHandle->fileName, "r+");
    if (pageNum > fHandle->totalNumPages) {
        fclose(fp);
        return RC_READ_NON_EXISTING_PAGE;
    }
    if (fseek(fp, pageNum * PAGE_SIZE, SEEK_SET) != 0) {
        fclose(fp);
        return RC_WRITE_FAILED;
    }
    if (fwrite(memPage, 1, PAGE_SIZE, fp) != PAGE_SIZE) {
        fclose(fp);
        return RC_WRITE_FAILED;
    }
    if (fHandle->totalNumPages == pageNum) {
        fHandle->totalNumPages++;
    }
    fHandle->curPagePos = pageNum;
    fclose(fp);
    return RC_OK;
}

RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    return writeBlock(fHandle->curPagePos, fHandle, memPage);
}

RC appendEmptyBlock(SM_FileHandle *fHandle) {
int flagSeek;
	size_t wSizeBlock;
	SM_PageHandle dummyBlock;
	dummyBlock=(SM_PageHandle)calloc(PAGE_SIZE,sizeof(char));
	flagSeek=fseek(fHandle->mgmtInfo,(fHandle->totalNumPages+1)*PAGE_SIZE*sizeof(char),SEEK_END);
	
    if(flagSeek==0){
		wSizeBlock=fwrite(dummyBlock,sizeof(char),PAGE_SIZE,fHandle->mgmtInfo);
		fHandle->totalNumPages=fHandle->totalNumPages+1;
		rewind(fHandle->mgmtInfo);  /* sets position to the startinf og the file */
		/* set position of file to starting again */
		fseek(fHandle->mgmtInfo,(fHandle->totalNumPages)*PAGE_SIZE*sizeof(char),SEEK_SET);
		free(dummyBlock);
		return RC_OK;
	}
	
    else{
		free(dummyBlock);
		return RC_WRITE_FAILED;
	}
}

RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle) {
int numPages,value=0;
	
    if(fHandle->totalNumPages<numberOfPages){
		numPages=numberOfPages-fHandle->totalNumPages;
	}
	
    while(value<numPages){
		appendEmptyBlock(fHandle);
		value++;	
	}
    return RC_OK;
}
