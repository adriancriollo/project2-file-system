// based on cs3650 starter code		#editing in nufs unlink function

static inode_t*
inode_from_num(int inum, int mode)
{
  inode_t* inode = block_read(inum);

  if (inode == NULL) {
    return NULL;
  }

  if (mode == S_IFDIR) {
    inode->type = 1;
  } else {
    inode->type = 0;
  }

  inode->refs++;

  return inode;
}


typedef struct {
  uint32_t inode_num;
  char name[DIR_NAME_LENGTH];
} dir_entry_t;

#define DIR_NAME_LENGTH 48
#define MAX_DIR_ENTRIES 16


typedef struct {
  int refs;
  uint32_t size;
  uint32_t type;      // 0 = file, 1 = directory
  uint32_t direct[6]; // Direct pointers
  uint32_t indirect;  // Indirect pointer
} inode_t;


#include <assert.h>
#include <bsd/string.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>

// implementation for: man 2 access
// Checks if a file exists.
int nufs_access(const char *path, int mask) {
  int rv = 0;

  // Only the root directory and our simulated file are accessible for now...
  if (strcmp(path, "/") == 0 || strcmp(path, "/hello.txt") == 0) {
    rv = 0;
  } else { // ...others do not exist
    rv = -ENOENT;
  }

  printf("access(%s, %04o) -> %d\n", path, mask, rv);
  return rv;
}

// Gets an object's attributes (type, permissions, size, etc).
// Implementation for: man 2 stat
// This is a crucial function.
int nufs_getattr(const char *path, struct stat *st) {
  int rv = 0;

  // Return some metadata for the root directory...
  if (strcmp(path, "/") == 0) {
    st->st_mode = 040755; // directory
    st->st_size = 0;
    st->st_uid = getuid();
  }
  // ...and the simulated file...
  else if (strcmp(path, "/hello.txt") == 0) {
    st->st_mode = 0100644; // regular file
    st->st_size = 6;
    st->st_uid = getuid();
  } else { // ...other files do not exist on this filesystem
    rv = -ENOENT;
  }
  printf("getattr(%s) -> (%d) {mode: %04o, size: %ld}\n", path, rv, st->st_mode,
         st->st_size);
  return rv;
}

// implementation for: man 2 readdir
// lists the contents of a directory
int nufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                 off_t offset, struct fuse_file_info *fi) {
  struct stat st;
  int rv;

  rv = nufs_getattr("/", &st);
  assert(rv == 0);

  filler(buf, ".", &st, 0);

  rv = nufs_getattr("/hello.txt", &st);
  assert(rv == 0);
  filler(buf, "hello.txt", &st, 0);

  printf("readdir(%s) -> %d\n", path, rv);
  return 0;
}

// mknod makes a filesystem object like a file or directory
// called for: man 2 open, man 2 link
// Note, for this assignment, you can alternatively implement the create
// function.
int nufs_mknod(const char *path, mode_t mode, dev_t rdev) {
  int rv = -1;
  printf("mknod(%s, %04o) -> %d\n", path, mode, rv);
  return rv;
}

// most of the following callbacks implement
// another system call; see section 2 of the manual
int nufs_mkdir(const char *path, mode_t mode) {
  int rv = nufs_mknod(path, mode | 040000, 0);
  printf("mkdir(%s) -> %d\n", path, rv);
  return rv;
}

int nufs_unlink(const char *path) {
  int rv = -1;

 #edit

    // Find the inode for the given path
    int inum = get_inode_of_path(path);
    if (inum < 0) {
        return -ENOENT; // File not found
    }

    // Decrement the link count of the inode
    struct inode* inode = get_inode(inum);
    inode->nlink--;

    // If the link count is now zero, free the inode and data blocks
    if (inode->nlink == 0) {
        free_inode(inum);
        free_data_blocks(inode->data_ptrs, inode->ind_ptrs);
    }

    // Update the parent directory's modification time
    update_modification_time(get_parent_path(path));

    // Remove the file from the parent directory
    remove_file_from_dir(get_parent_path(path), get_filename_from_path(path));

    return 0; // Success


  
  printf("unlink(%s) -> %d\n", path, rv);
  return rv;
}

int nufs_link(const char *from, const char *to) {
  int rv = -1;
  printf("link(%s => %s) -> %d\n", from, to, rv);
  return rv;
}

int nufs_rmdir(const char *path) {
  int rv = -1;
  printf("rmdir(%s) -> %d\n", path, rv);
  return rv;
}

// implements: man 2 rename
// called to move a file within the same filesystem
int nufs_rename(const char *from, const char *to) {
  int rv = -1;
  printf("rename(%s => %s) -> %d\n", from, to, rv);
  return rv;
}

int nufs_chmod(const char *path, mode_t mode) {
  int rv = -1;
  printf("chmod(%s, %04o) -> %d\n", path, mode, rv);
  return rv;
}

int nufs_truncate(const char *path, off_t size) {
  int rv = -1;
  printf("truncate(%s, %ld bytes) -> %d\n", path, size, rv);
  return rv;
}

// This is called on open, but doesn't need to do much
// since FUSE doesn't assume you maintain state for
// open files.
// You can just check whether the file is accessible.
int nufs_open(const char *path, struct fuse_file_info *fi) {
  int rv = 0;
  printf("open(%s) -> %d\n", path, rv);
  return rv;
}

int
nufs_rename(const char *path, const char *newpath)
{
 

  // Make sure the new path doesn't already exist
  if (get_inode_from_path(newpath) != NULL) {
    return -EEXIST;
  }

  // Get the inode of the directory being renamed
  inode_t* inode = get_inode_from_path(path);
  if (inode == NULL) {
    return -ENOENT;
  }

  // Get the parent directory's inode
  char* parent_path = get_parent_path(newpath);
  inode_t* parent_inode = get_inode_from_path(parent


int
nufs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{

  if (mode == S_IFDIR) {
    int inum = alloc_inode();
    if (inum == 0) {
      return -errno;
    }

    // Create the new directory inode
    inode_t* inode = inode_from_num(inum, mode);
    if (inode == NULL) {
      free_inode(inum);
      return -errno;
    }

    // Create the "." directory entry
    dir_entry_t dot_entry;
    dot_entry.inode_num = inum;
    strcpy(dot_entry.name, ".");
    write_dir_entry(parent_inode, &dot_entry);

    // Create the ".." directory entry
    dir_entry_t dotdot_entry;
    dotdot_entry.inode_num = parent_inode_num;
    strcpy(dotdot_entry.name, "..");
    write_dir_entry(inode, &dotdot_entry);

    return 0;
  }

}


// Actually read data
int nufs_read(const char *path, char *buf, size_t size, off_t offset,
              struct fuse_file_info *fi) {
  int rv = 6;
  strcpy(buf, "hello\n");
  printf("read(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
  return rv;
}

// Actually write data
int nufs_write(const char *path, const char *buf, size_t size, off_t offset,
               struct fuse_file_info *fi) {
  int rv = -1;
  printf("write(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
  return rv;
}

// Update the timestamps on a file or directory.
int nufs_utimens(const char *path, const struct timespec ts[2]) {
  int rv = -1;
  printf("utimens(%s, [%ld, %ld; %ld %ld]) -> %d\n", path, ts[0].tv_sec,
         ts[0].tv_nsec, ts[1].tv_sec, ts[1].tv_nsec, rv);
  return rv;
}

// Extended operations
int nufs_ioctl(const char *path, int cmd, void *arg, struct fuse_file_info *fi,
               unsigned int flags, void *data) {
  int rv = -1;
  printf("ioctl(%s, %d, ...) -> %d\n", path, cmd, rv);
  return rv;
}

void nufs_init_ops(struct fuse_operations *ops) {
  memset(ops, 0, sizeof(struct fuse_operations));
  ops->access = nufs_access;
  ops->getattr = nufs_getattr;
  ops->readdir = nufs_readdir;
  ops->mknod = nufs_mknod;
  // ops->create   = nufs_create; // alternative to mknod
  ops->mkdir = nufs_mkdir;
  ops->link = nufs_link;
  ops->unlink = nufs_unlink;
  ops->rmdir = nufs_rmdir;
  ops->rename = nufs_rename;
  ops->chmod = nufs_chmod;
  ops->truncate = nufs_truncate;
  ops->open = nufs_open;
  ops->read = nufs_read;
  ops->write = nufs_write;
  ops->utimens = nufs_utimens;
  ops->ioctl = nufs_ioctl;
};

struct fuse_operations nufs_ops;

int main(int argc, char *argv[]) {
  assert(argc > 2 && argc < 6);
  printf("TODO: mount %s as data file\n", argv[--argc]);
  // storage_init(argv[--argc]);
  nufs_init_ops(&nufs_ops);
  return fuse_main(argc, argv, &nufs_ops, NULL);
}
