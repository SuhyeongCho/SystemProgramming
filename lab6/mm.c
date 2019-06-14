/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
	/* Team name */
	"ateam",
	/* First member's full name */
	"Harry Bovik",
	/* First member's email address */
	"bovik@cs.cmu.edu",
	/* Second member's full name (leave blank if none) */
	"",
	/* Second member's email address (leave blank if none) */
	""
};

#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12)
/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))


#define PACK(size,alloc) ((size)|(alloc))
#define GET(p) (*(unsigned *)(p))
#define PUT(p,val) (*(unsigned *)(p) = (val))
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

#define HEADER(bp) ((char *)(bp) - WSIZE)
#define FOOTER(bp) ((char *)(bp) + GET_SIZE(HEADER(bp)) - DSIZE)

#define NEXT_BLOCK(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLOCK(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

#define NEXT_FREE_BLOCK(bp) (*(void **)(bp))
#define PREV_FREE_BLOCK(bp) (*(void **)(bp + WSIZE))
/* 
 * mm_init - initialize the malloc package.
 */
static void* extend_heap(size_t);
static void* coalesce(void*);

void* find_fit(size_t);
void place(void*, size_t);

void addFreeBlock(void*);
void delFreeBlock(void*);

void *heap_list;
void *root;
void *seg[5];

int mm_init(void)
{
	if ( (heap_list = mem_sbrk(4*WSIZE)) == (void*)-1) return -1;

	PUT(heap_list,0);
	PUT(heap_list + 1*WSIZE, PACK(DSIZE,1));
	PUT(heap_list + 2*WSIZE, PACK(DSIZE,1));
	PUT(heap_list + 3*WSIZE, PACK(0,1));
	
	heap_list += DSIZE;
	root = NULL;

	if(extend_heap(CHUNKSIZE/WSIZE) == NULL) return -1;
	return 0;
}

static void* extend_heap(size_t size) {
	void *bp;
	size_t asize;

	asize = (size % 2) ? (size + 1)*WSIZE : size*WSIZE;

	if((bp = mem_sbrk(asize)) == (void*)-1) return NULL;

	PUT(HEADER(bp), PACK(asize, 0));
	PUT(FOOTER(bp), PACK(asize, 0));
	PUT(HEADER(NEXT_BLOCK(bp)), PACK(0,1));

	return coalesce(bp);
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
	void *bp;
	size_t newsize, extendsize;
	if (size <= 0) return NULL;
	if (size <= DSIZE) newsize = 2*DSIZE;
	else newsize = ALIGN(size + SIZE_T_SIZE);

	if ( (bp = find_fit(newsize)) != NULL) {
		place(bp, newsize);
	} else {
		extendsize = ( (newsize > CHUNKSIZE) ? newsize : CHUNKSIZE );
		if( (bp = extend_heap(extendsize/WSIZE) ) == NULL) return NULL;
		place(bp, newsize);
	}
	return bp;
}

void *find_fit(size_t size) {
	void *bp;
	for(bp = root; bp != NULL; bp = (void *)NEXT_FREE_BLOCK(bp)) {
		if (!GET_ALLOC(HEADER(bp)) && (size <= GET_SIZE(HEADER(bp)))) {
			return bp;
		}
	}
	return NULL;
}

void place(void *bp, size_t size) {
	size_t asize = GET_SIZE(HEADER(bp));

	if(asize - size >= DSIZE*2) {
		delFreeBlock(bp);
		PUT(HEADER(bp), PACK(size, 1));
		PUT(FOOTER(bp), PACK(size, 1));

		bp = NEXT_BLOCK(bp);
		
		PUT(HEADER(bp), PACK(asize - size, 0));
		PUT(FOOTER(bp), PACK(asize - size, 0));
		addFreeBlock(bp);
	} else {
		PUT(HEADER(bp), PACK(asize, 1));
		PUT(FOOTER(bp), PACK(asize, 1));
		delFreeBlock(bp);
	}
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
	size_t asize = GET_SIZE(HEADER(ptr));

	PUT(HEADER(ptr), PACK(asize, 0));
	PUT(FOOTER(ptr), PACK(asize, 0));

	coalesce(ptr);

}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
	if (ptr == NULL && size != 0) {
		void *mm = mm_malloc(size);
		return mm;
	}
	if (size == 0) {
		mm_free(ptr);
		return NULL;
	}
	void *oldptr = ptr;
	void *newptr;
	size_t copySize;

	newptr = mm_malloc(size);
	if (newptr == NULL)
		return NULL;
	copySize = *(size_t *)((char *)oldptr - WSIZE);
	if (size < copySize)
		copySize = size;
	memcpy(newptr, oldptr, copySize);
	mm_free(oldptr);
	return newptr;
}
static void *coalesce(void *bp) {
	size_t prev_alloc = GET_ALLOC(FOOTER(PREV_BLOCK(bp)));
	size_t next_alloc = GET_ALLOC(HEADER(NEXT_BLOCK(bp)));
	size_t asize = GET_SIZE(HEADER(bp));

	if (prev_alloc && next_alloc) {
		addFreeBlock(bp);
	} else if (prev_alloc && !next_alloc) {
		delFreeBlock(NEXT_BLOCK(bp));
		asize += GET_SIZE(HEADER(NEXT_BLOCK(bp)));
		PUT(HEADER(bp), PACK(asize, 0));
		PUT(FOOTER(bp), PACK(asize, 0));
		addFreeBlock(bp);
	} else if (!prev_alloc && next_alloc) {
		delFreeBlock(PREV_BLOCK(bp));
		asize += GET_SIZE(HEADER(PREV_BLOCK(bp)));
		PUT(FOOTER(bp), PACK(asize, 0));
		PUT(HEADER(PREV_BLOCK(bp)), PACK(asize, 0));
		bp = PREV_BLOCK(bp);
		addFreeBlock(bp);
	} else {
		delFreeBlock(PREV_BLOCK(bp));
		delFreeBlock(NEXT_BLOCK(bp));
		asize += GET_SIZE(HEADER(PREV_BLOCK(bp)));
		asize += GET_SIZE(HEADER(NEXT_BLOCK(bp)));
		PUT(HEADER(PREV_BLOCK(bp)), PACK(asize, 0));
		PUT(FOOTER(NEXT_BLOCK(bp)), PACK(asize, 0));
		bp = PREV_BLOCK(bp);
		addFreeBlock(bp);
	}
	return bp;
}
void addFreeBlock(void *bp) {
	if (root == NULL) {
		root = bp;
		NEXT_FREE_BLOCK(bp) = NULL;
		PREV_FREE_BLOCK(bp) = NULL;

	} else {
		void* tmp = root;
		root = bp;
		NEXT_FREE_BLOCK(bp) = tmp;
		PREV_FREE_BLOCK(bp) = NULL;
		PREV_FREE_BLOCK(tmp) = bp;
	}
}

void delFreeBlock(void *bp) {
	void *prev = PREV_FREE_BLOCK(bp);
	void *next = NEXT_FREE_BLOCK(bp);
	
	if (prev == NULL) {
		root = next;
	} else {
		NEXT_FREE_BLOCK(prev) = next;
	}
	if (next != NULL) {
		PREV_FREE_BLOCK(next) = prev;
	}
}












