#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int h1;
    char h2;
    double h3;
    char h4;
    char *h5; 
} my_page_t;


int main(int argc, char *argv[]) {
  my_page_t *chunks, *buf_chunks;
  my_page_t *to_free_chunk;
  int i;
  int n_chunks = 3;

  buf_chunks = (my_page_t *)malloc( n_chunks * sizeof(my_page_t));
  to_free_chunk = buf_chunks + 1;
  

  for ( i = 0; i < n_chunks; ++i) {
     buf_chunks[i].h3 = (double) i + 1;
  }
    chunks = (my_page_t *) malloc((n_chunks - 1) * sizeof *chunks);
	    memcpy(chunks, buf_chunks,
	       (to_free_chunk - buf_chunks) * sizeof *chunks);
	    memcpy(chunks + (to_free_chunk - buf_chunks),
	           to_free_chunk + 1,
	           ( buf_chunks + n_chunks
	           - (to_free_chunk + 1) ));
    free(buf_chunks);
    buf_chunks = chunks;
    n_chunks --;

  for ( i = 0; i < n_chunks; ++i) {
     printf("\n Elem %d is %.2f", i, buf_chunks[i].h3);
  }

  return 0;
}
