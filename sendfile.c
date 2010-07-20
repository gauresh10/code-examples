#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/sendfile.h>

int main(int argc, char **argv) {
    int src;               /* file descriptor for source file */
    int dest;              /* file descriptor for destination file */
    struct stat stat_buf;  /* hold information about input file */
    off_t offset = 0;      /* byte offset used by sendfile */

    /* check that source file exists and can be opened */
    src = open(argv[1], O_RDONLY);

    if (src < 0) {
        fprintf(stderr, "open call failed with errno '%s'\n", strerror(errno));
    }

    /* get size and permissions of the source file */
    if (fstat(src, &stat_buf) < 0) {
        fprintf(stderr, "fstat call failed with errno '%s'\n", strerror(errno));
    }

    /* open destination file */
    dest = open(argv[2], O_WRONLY|O_CREAT, stat_buf.st_mode);

    if (dest < 0) {
        fprintf(stderr, "open call failed with errno '%s'\n", strerror(errno));
    }

    fprintf(stdout, "Sendfile (%d, %d, %u, %u)\n", dest, src, offset, stat_buf.st_size);

    /* copy file using sendfile */
    if (sendfile (dest, src, &offset, stat_buf.st_size) < 0) {
        fprintf(stderr, "Sendfile call failed with errno '%s'\n", strerror(errno));
    }

    /* clean up and exit */
    close(dest);
    close(src);

    return 0;
}

