#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/queue.h>

//#define _GNU_SOURCE
#define __USE_GNU
#include <sys/mman.h>

#define UNIV_PAGE_SIZE 1024
#define SAMPLE_SOURCE_FILE "./mremap_sample.txt"
//#define fprintf(...) ;
//#define fflush(...) ;


/* typedefs */

typedef unsigned long int ulint;
typedef unsigned char byte;

typedef struct buf_page_t {
    byte *frame;
    LIST_ENTRY(buf_page_t) list;
} buf_page_t;

typedef struct {
    int   block_t;
    int   block_status;
    buf_page_t page;
    byte *frame;
} buf_block_t;

typedef struct {
    ulint   mem_size;
    ulint   size;
    byte   *mem;
    buf_block_t *blocks;
} buf_chunk_t;

typedef LIST_HEAD(page_list_head_t, buf_page_t) page_list_head_t;

typedef struct {
    page_list_head_t free;
} buf_pool_t;

buf_pool_t *buf_pool;
char *text;

/* round-up and alignment */
#define ut_2pow_round(n, m) ((n) & ~((m) - 1))
void*
ut_align(
        const void*     ptr,            /*!< in: pointer */
        ulint           align_no)       /*!< in: align by this number */
{
        assert(align_no > 0);
        assert(((align_no - 1) & align_no) == 0);
        assert(ptr);

        assert(sizeof(void*) == sizeof(ulint));

        return((void*)((((ulint)ptr) + align_no - 1) & ~(align_no - 1)));
}

/* mmap */
void *pool_map(ulint size)
{
    void *ptr = NULL;
    ptr = mmap(NULL, size, PROT_READ | PROT_WRITE,
		   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return ptr;
}

void *pool_remap(void *ptr, ulint old_size, ulint size)
{
	ptr = mremap(ptr, old_size, size, MREMAP_MAYMOVE);
    return ptr;
}

void pool_destroy(void *ptr, ulint size)
{
    munmap(ptr, size);
}

/* list */
void print_list(page_list_head_t *free)
{
    buf_page_t *elem;
    ulint i = 0;

    LIST_FOREACH(elem, free, list) {
        fprintf(stdout, "%c", elem->frame[0]);
        ++i;
    }

        fprintf(stdout, "\n\n %ld ", i);
}

/* pool */
void buf_block_init(buf_block_t *block, byte *frame)
{
    static ulint pos = 0;

    block->page.frame = frame;
    block->frame = frame;
    if (pos >= 1024 * 10) { pos = 0; }
    block->frame[0] = text[pos++];
}

buf_chunk_t *pool_init(buf_chunk_t *chunk, ulint mem_size);
buf_chunk_t *pool_resize(buf_chunk_t *chunk, ulint mem_size);

char *read_file(char *sample)
{
    FILE *f;
    char *ret, *init;
    ulint max_len, read_len = 1024 * 10;

    max_len = read_len;
    init = sample = malloc(sizeof(char) * max_len);

    f = fopen(SAMPLE_SOURCE_FILE, "r");
    while ( (ret = fgets(sample, read_len, f)) != NULL && read_len > 1) {
        read_len -= strlen(sample);
        sample += strlen(sample);
    }

    fprintf(stdout, "Read file\n");
    fflush(stdout);
    return init;
}

int main(int argc, char *argv[])
{
    byte *old_mem_p, *mem_p;

    ulint size_init = 11 * 1024 * 1024;
    ulint size_map_to = 5 * 1024 * 1024;
    buf_chunk_t *pool = (buf_chunk_t *) malloc(sizeof(buf_chunk_t));
    buf_pool = (buf_pool_t *) malloc(sizeof(buf_pool_t));

    text = read_file(text);

    LIST_INIT( &buf_pool->free );
    pool_init(pool, size_init);
    fprintf(stdout, "\nMemory mapped at %p\n", pool->mem);
    fflush(stdout);

    //traverse a list
    print_list( &buf_pool->free );

    pool = pool_resize(pool, size_map_to);
    fprintf(stdout, "\nMemory remapped at %p\n", pool->mem);
    fflush(stdout);

    //traverse a list
    print_list( &buf_pool->free );
    pool_destroy(pool->mem, size_map_to);
    return 0;
}


buf_chunk_t *pool_init(buf_chunk_t *chunk, ulint mem_size)
{
    byte *frame;
    buf_block_t *block;
    ulint i;

	chunk->mem_size = ut_2pow_round(mem_size, UNIV_PAGE_SIZE);
	/* Reserve space for the block descriptors. */
	chunk->mem_size += ut_2pow_round((mem_size / UNIV_PAGE_SIZE) * (sizeof *block)
				  + (UNIV_PAGE_SIZE - 1), UNIV_PAGE_SIZE);
    chunk->mem = (byte *) pool_map(chunk->mem_size);
    //allocate descriptors

    chunk->blocks = (buf_block_t *) chunk->mem;

	frame = (byte *)ut_align(chunk->mem, UNIV_PAGE_SIZE);
	chunk->size = chunk->mem_size / UNIV_PAGE_SIZE
		- (frame != chunk->mem);

	/* Subtract the space needed for block descriptors. */
	{
		ulint	size = chunk->size;

		while (frame < (byte*) (chunk->blocks + size)) {
			frame += UNIV_PAGE_SIZE;
			size--;
		}

		chunk->size = size;
	}

	block = chunk->blocks;
	for (i = chunk->size; i--; ) {

        //init them
		buf_block_init(block, frame);
         //add to list
        LIST_INSERT_HEAD(&buf_pool->free, &block->page, list);

		block++;
		frame += UNIV_PAGE_SIZE;
	}

	return(chunk);
}

buf_chunk_t *pool_resize(buf_chunk_t *chunk, ulint mem_size)
{
	buf_block_t	*block;
	byte		*frame;
	byte		*old_mem_addr, *start_move_frame;
	ulint		i;
    ulint       old_chunk_size, new_size;
    ulint       old_mem_size, moved_size;

	/* Round down to a multiple of page size,
	although it already should be. */
	mem_size = ut_2pow_round(mem_size, UNIV_PAGE_SIZE);
	/* Reserve space for the block descriptors. */
	mem_size += ut_2pow_round((mem_size / UNIV_PAGE_SIZE) * (sizeof *block)
				  + (UNIV_PAGE_SIZE - 1), UNIV_PAGE_SIZE);


    /* Save the old values */
    old_mem_size = chunk->mem_size;
    old_mem_addr = chunk->mem;
    frame = ut_align(chunk->mem, UNIV_PAGE_SIZE);
    old_chunk_size = chunk->size;

    /* Calculate new frame address */
    start_move_frame = chunk->blocks[0].frame;

    frame = ut_align(chunk->mem, UNIV_PAGE_SIZE);

    {
	    new_size = mem_size / UNIV_PAGE_SIZE
	    - (frame != chunk->mem);

	    while (frame < (byte*) (chunk->blocks + new_size)) {
		    frame += UNIV_PAGE_SIZE;
		    new_size--;
	    }
    }

    fprintf(stdout, "\nmem_size %ld < old_mem_size %ld", mem_size, old_mem_size);

    fprintf(stderr, "\n mem at %p size %ld frame is %p start_move_frame %p\n", chunk->mem, chunk->size, frame, start_move_frame);

    if (mem_size < old_mem_size) {

        ulint i = 0;
        for ( block = chunk->blocks + new_size; block < chunk->blocks + old_chunk_size; block++ ) {
            //if (block->frame < size)
		    //ut_a(buf_block_get_state(block) == BUF_BLOCK_NOT_USED);
            //if (block->page.in_free_list) {
		        LIST_REMOVE((&block->page), list);
            //} else {
            //    fpintf(stderr, "Block in_flush: %d,in_unzip_LRU: %d,in_LRU: %d\n", block->page.in_flush_list, block->in_unzip_LRU_list, block->page.in_LRU_list);
            //}
            i++;
        }

        fprintf(stdout, "\n removed %ld items", i);
        /* Move the memory content before resizing */
        memmove(frame, start_move_frame, UNIV_PAGE_SIZE * chunk->size);

    }

	chunk->mem  = pool_remap(chunk->mem, chunk->mem_size, mem_size);
    chunk->mem_size = mem_size;

    /* reallocation success check 
	if (UNIV_UNLIKELY(chunk->mem_size == old_mem_size)) {

		return(NULL);
	}
     */

    if (mem_size > old_mem_size) {

		fprintf(stderr, "\n moving higher %p to %p, %ld, %ld size %d\n", 
//chunk->mem + (old_mem_addr - frame), chunk->mem + (old_mem_addr - start_move_frame), (frame - old_mem_addr), (old_mem_addr - chunk->mem), chunk->size);
chunk->mem + (frame - old_mem_addr), chunk->mem + (start_move_frame - old_mem_addr), (frame - old_mem_addr), (old_mem_addr - chunk->mem), chunk->size);
        /* Move the memory content */
        memmove(chunk->mem + (frame - old_mem_addr),
            chunk->mem + (start_move_frame - old_mem_addr), 
            UNIV_PAGE_SIZE * chunk->size);
    }

	/* Rearrange the memory */

	frame = ut_align(chunk->mem, UNIV_PAGE_SIZE);

    chunk->mem_size = mem_size;
	chunk->size = chunk->mem_size / UNIV_PAGE_SIZE
		- (frame != chunk->mem);

	chunk->blocks = chunk->mem;

	/* Subtract the space needed for block descriptors. */
	{
		ulint	size = chunk->size;

		while (frame < (byte*) (chunk->blocks + size)) {
			frame += UNIV_PAGE_SIZE;
			size--;
		}

		chunk->size = size;
	}

    block = chunk->blocks;

    i = old_chunk_size > chunk->size ? chunk->size : old_chunk_size;

    while (i--) {

		block->frame = frame;
        block->page.frame = frame;
        if ((block->page.list).le_prev) {
            (block->page.list).le_prev = (buf_page_t *)( ((byte *) (block->page.list).le_prev )+ (chunk->mem - old_mem_addr));
        }
        if ((block->page.list).le_next) {
            (block->page.list).le_next = (buf_page_t *)( ((byte *) (block->page.list).le_next )+ (chunk->mem - old_mem_addr));
        }
	    block++;
	    frame += UNIV_PAGE_SIZE;

    }

    fprintf(stderr, "\n %ld", i);

    buf_pool->free.lh_first = (buf_page_t *)( ((byte *) buf_pool->free.lh_first )+ (chunk->mem - old_mem_addr));

	/* Init block structs if the new size is larger. */
	for (i = chunk->size; i > old_chunk_size; i--) {

		buf_block_init(block, frame);
        memset(block->frame, '\0', UNIV_PAGE_SIZE);

		/* Add the block to the free list */
        LIST_INSERT_HEAD(&buf_pool->free, &block->page, list);

		block++;
		frame += UNIV_PAGE_SIZE;
	}

    fprintf(stderr, "\n %ld", i);

    return chunk;
}

