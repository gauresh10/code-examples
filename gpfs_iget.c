#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <gpfs.h>

#define MAX_INODE_INCREMENT 1000
//static int numInodes = 0;
//static int maxInodes = 0;
//static ino_t *Inode = NULL;

/* Read all entries from the current directory */
int read_dir(const char *pathP, char *RootFsDirP, ino_t inode)
{
  int rc = 0, ind;
  const gpfs_direntx_t *direntxP;
  gpfs_fssnap_handle_t *fsP = NULL;
  gpfs_ifile_t *dirxP = NULL;
  char *typeP, *slashP;

  /* Check if we need a "/"  */
  if (strlen(pathP) > 0)
    slashP = "/";
  else
    slashP = "";

  /* Open the file system */
  fsP = gpfs_get_fssnaphandle_by_path(RootFsDirP);
  if (fsP == NULL)
  {
    rc = errno;
    fprintf(stderr, "gpfs_get_fssnaphandle_by_path(%s): %s\n",
            RootFsDirP, strerror(rc));
    goto exit;
  }

  /* If there is only one inode in the list and is one of the reserved inode
     it cannot possibly be found, so stop the nonsense now.
     Calling tsfindinode -i 1 can be a fast test as to whether the filesystem
     is mounted and ready to use. */
  // if (numInodes == 1 && Inode[0] < 3)
    // goto exit;

  /* Open the directory's file */
  dirxP = gpfs_iopen(fsP, inode, O_RDONLY, NULL, NULL);
  if (dirxP == NULL)
  {
    rc = errno;
    fprintf(stderr, "gpfs_iopen(%s): %s\n", pathP, strerror(rc));
    goto exit;
  }

  fprintf(stdout, "gpfs_iopen(%s): %d\n", pathP, dirxP);

  /* Loop reading the directory entries */
  while (1)
  {
    rc = gpfs_ireaddir(dirxP, &direntxP);
    if (rc != 0)
    {
      int saveerrno = errno;
      fprintf(stderr, "gpfs_ireaddir(%s): rc %d %s\n",
              pathP, rc, strerror(saveerrno));
      rc = saveerrno;
      goto exit;
    }

    if (direntxP == NULL)
      break;

    printf("%10d\t%s%s%s\n",
                     direntxP->d_ino, pathP, slashP, direntxP->d_name);

    /* Get directory entry type */
    switch (direntxP->d_type)
    {
      case GPFS_DE_DIR:
        /* Skip . and .. entries */
        if ((strcmp(direntxP->d_name, ".") == 0) ||
            (strcmp(direntxP->d_name, "..") == 0))
          continue;

        typeP = "DIR";
        break;

      case GPFS_DE_REG:
        typeP = "REG";
        break;

      case GPFS_DE_LNK:
        typeP = "LNK";
        break;

      default:
        typeP = "UNK";
    }

    // ind = testinode(direntxP->d_ino);
    // if (ind >= 0)
    // {
      // int plen;

      // /* Print: inodenumber pathname */
      // plen = printf("%10d\t%s%s%s%s",
                    // direntxP->d_ino, pathP, slashP, direntxP->d_name, print0);
      // InodeSeen[ind] = 1;
      // if (plen < 0)
      // {
        // rc = errno;
        // goto exit;
      // }
    // }

    // printf("%10d\t%s%s%s\n",
                     // direntxP->d_ino, pathP, slashP, direntxP->d_name);
    /* Check if this entry is a directory */
    if (direntxP->d_type == GPFS_DE_DIR)
    {
    }
  }


exit:
  /* Close open file, if necessary */
  if (dirxP)
    gpfs_iclose(dirxP);
  if (fsP)
    gpfs_free_fssnaphandle(fsP);

  return rc;
}


int main(int argc, char **argv)
{
    char *RootFsDirP = "/gpfs";
    unsigned long int inode;
    if (argc > 1) {
            inode = atoi(argv[1]);
    }

    read_dir("", RootFsDirP, inode);
    return 0;
}