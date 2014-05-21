/*
 * CS3600, Spring 2014
 * Project 2 Starter Code
 * (c) 2013 Alan Mislove
 *
 * This file contains all of the basic functions that you will need 
 * to implement for this project.  Please see the project handout
 * for more details on any particular function, and ask on Piazza if
 * you get stuck.
 */

 /* 
  * Written by Team MuffinButton
  * Edwin Cowart & Cody Wetherby
  * on 2014/02/12
  */

#define FUSE_USE_VERSION 26

#ifdef linux
/* For pread()/pwrite() */
#define _XOPEN_SOURCE 500
#endif

#define _POSIX_C_SOURCE 199309



#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

#include "3600fs.h"

#define USE(x) (x) = (x)

/* Is the vcb we have valid? */
bool vcb_valid = false;

/* The volume control block of the partition. */
vcb drive;

/* The root directory of the disk. */
dnode root;

/* The Red-Black Tree - Linked List which is a cache of blocknums and the blocks. */
rbtreelist cache;


/*
 * Initialize filesystem. Read in file system metadata and initialize
 * memory structures. If there are inconsistencies, now would also be
 * a good time to deal with that. 
 *
 * HINT: You don't need to deal with the 'conn' parameter AND you may
 * just return NULL.
 *
 */
// Milestone 2
static void* vfs_mount(struct fuse_conn_info *conn) {
  USE(conn);
  fprintf(stderr, "vfs_mount called\n");

  // Do not touch or move this code; connects the disk
  dconnect();

  /* 3600: YOU SHOULD ADD CODE HERE TO CHECK THE CONSISTENCY OF YOUR DISK
           AND LOAD ANY DATA STRUCTURES INTO MEMORY */
  char vcb_char[512];
  int vcb_read = dread2(VCB_BLOCKNUM, vcb_char);
  if (vcb_read < 0) {
    printf("Error: Could not read vcb.\n");
  } else if (vcb_read != BLOCKSIZE) {
    printf("Error: Wrong number of bytes read. Expected %i, read %i.\n", BLOCKSIZE, vcb_read);
  } else {
    drive = (*(vcb*)vcb_char);
    if (drive.magic != MAGICNUM) {
      printf("Error: Woops, wrong drive.\n");
      vcb_valid = false;
    } else {
      vcb_valid = true;
    }
  }
  char root_char[512];
  int root_read = dread2(ROOT_BLOCKNUM, root_char);
  if (root_read < 0) {
    printf("Error: Could not read root.\n");
    vcb_valid = false;
  } else if (root_read != BLOCKSIZE) {
    printf("Error: Wrong number of bytes read. Expected %i, read %i.\n", BLOCKSIZE, root_read);
    vcb_valid = false;
  } else {
    root = (*(dnode*)root_char);
  }
  cache = {0, 0, 0, NULL, NULL, NULL};
  return NULL;
}

/*
 * Called when your file system is unmounted.
 */
static void vfs_unmount (void *private_data) {
  USE(private_data);
  fprintf(stderr, "vfs_unmount called\n");

  /* 3600: YOU SHOULD ADD CODE HERE TO MAKE SURE YOUR ON-DISK STRUCTURES
           ARE IN-SYNC BEFORE THE DISK IS UNMOUNTED (ONLY NECESSARY IF YOU
           KEEP DATA CACHED THAT'S NOT ON DISK */
  vertex* next = NULL;
  for (vertex* vtx = cache.head; vtx != NULL; vtx = next) {
    next = vtx->next;
    if (vtx->writes > 0) {
      dwrite2(vtx->disk_index, vtx->block);
    }
    free(vtx->block);
    free(vtx);
  }

  // Do not touch or move this code; unconnects the disk
  dunconnect();
}

/* 
 * Given an absolute path to a file/directory (i.e., /foo ---all
 * paths will start with the root directory of the CS3600 file
 * system, "/"), you need to return the file attributes that is
 * similar stat system call.
 *
 * HINT: You must implement stbuf->stmode, stbuf->st_size, and
 * stbuf->st_blocks correctly.
 *
 */
// Milestone 2
static int vfs_getattr(const char *path, struct stat *stbuf) {
  USE(path);
  USE(stbuf);
  fprintf(stderr, "vfs_getattr called\n");

  char** pathtoks = parsepath(path);
  // Get parent (always a dnode 'cause directory) 
    // Use iterator to get to parent
  // Get type of child
  // Get child, cast as type stored in parent
  /*FILETYPE*/ child = (/*FILETYPE*/) read_block(/*CHILDBLOCKNUM*/);

  // Do not mess with this code 
  stbuf->st_nlink = 1; //we have a hard link tracker, so we're using that instead
  stbuf->st_rdev  = 0;
  stbuf->st_blksize = BLOCKSIZE;

  if ()
    stbuf->st_mode  = 0777 | S_IFDIR;
  else 
    stbuf->st_mode  = <<file mode>> | S_IFREG;

  stbuf->st_uid     = // file uid
  stbuf->st_gid     = // file gid
  stbuf->st_atime   = // access time 
  stbuf->st_mtime   = // modify time
  stbuf->st_ctime   = // create time
  stbuf->st_size    = // file size
  stbuf->st_blocks  = // file size in blocks

  return 0;
}

/*
 * Given an absolute path to a directory (which may or may not end in
 * '/'), vfs_mkdir will create a new directory named dirname in that
 * directory, and will create it with the specified initial mode.
 *
 * HINT: Don't forget to create . and .. while creating a
 * directory.
 *
 * path: The path of the directory being created
 * mode: The initial mode (permission) of the directory
 * return value: 0 if successful, -1 if an error occurred
 */
static int vfs_mkdir(const char *path, mode_t mode) {
  USE(path);
  USE(mode);

  return -1;
}

/* Read directory
 *
 * Given an absolute path to a directory, vfs_readdir will return 
 * all the files and directories in that directory.
 *
 * HINT:
 * Use the filler parameter to fill in, look at fusexmp.c to see an example
 * Prototype below
 *
 * Function to add an entry in a readdir() operation
 *
 * @param buf the buffer passed to the readdir() operation
 * @param name the file name of the directory entry
 * @param stat file attributes, can be NULL
 * @param off offset of the next entry or zero
 * @return 1 if buffer is full, zero otherwise
 * typedef int (*fuse_fill_dir_t) (void *buf, const char *name,
 *                                 const struct stat *stbuf, off_t off);
 *         
 * Your solution should not need to touch fi
 *
 */
// Milestone 2
static int vfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi) {
    USE(path);
    USE(buf);
    USE(filler);
    USE(offset);
    USE(fi);
    return 0;
}

/*
 * Given an absolute path to a file (for example /a/b/myFile), vfs_create 
 * will create a new file named myFile in the /a/b directory.
 *
 */
// Milestone 2
static int vfs_create(const char *path, mode_t mode, 
    struct fuse_file_info *fi) {
  USE(path);
  USE(mode);
  USE(fi);
  return 0;
}

/*
 * The function vfs_read provides the ability to read data from 
 * an absolute path 'path,' which should specify an existing file.
 * It will attempt to read 'size' bytes starting at the specified
 * offset (offset) from the specified file (path)
 * on your filesystem into the memory address 'buf'. The return 
 * value is the amount of bytes actually read; if the file is 
 * smaller than size, vfs_read will simply return the most amount
 * of bytes it could read. 
 *
 * HINT: You should be able to ignore 'fi'
 *
 */
static int vfs_read(const char *path, char *buf, size_t size, off_t offset,
                    struct fuse_file_info *fi) {
    USE(path);
    USE(buf);
    USE(size);
    USE(offset);
    USE(fi);
    return 0;
}

/*
 * The function vfs_write will attempt to write 'size' bytes from 
 * memory address 'buf' into a file specified by an absolute 'path'.
 * It should do so starting at the specified offset 'offset'.  If
 * offset is beyond the current size of the file, you should pad the
 * file with 0s until you reach the appropriate length.
 *
 * You should return the number of bytes written.
 *
 * HINT: Ignore 'fi'
 */
static int vfs_write(const char *path, const char *buf, size_t size,
                     off_t offset, struct fuse_file_info *fi) {
  USE(path);
  USE(buf);
  USE(size);
  USE(offset);
  USE(fi);
  /* 3600: NOTE THAT IF THE OFFSET+SIZE GOES OFF THE END OF THE FILE, YOU
           MAY HAVE TO EXTEND THE FILE (ALLOCATE MORE BLOCKS TO IT). */

  return 0;
}

/*
 * This function deletes the last component of the path (e.g., /a/b/c you 
 * need to remove the file 'c' from the directory /a/b).
 */
// Milestone 2
static int vfs_delete(const char *path) {
  USE(path);
  /* 3600: NOTE THAT THE BLOCKS CORRESPONDING TO THE FILE SHOULD BE MARKED
           AS FREE, AND YOU SHOULD MAKE THEM AVAILABLE TO BE USED WITH OTHER FILES */

    return 0;
}

/*
 * The function rename will rename a file or directory named by the
 * string 'oldpath' and rename it to the file name specified by 'newpath'.
 *
 * HINT: Renaming could also be moving in disguise
 *
 */
static int vfs_rename(const char *from, const char *to) {
    USE(from);
    USE(to);
    return 0;
}


/*
 * This function will change the permissions on the file
 * to be mode.  This should only update the file's mode.  
 * Only the permission bits of mode should be examined 
 * (basically, the last 16 bits).  You should do something like
 * 
 * fcb->mode = (mode & 0x0000ffff);
 *
 */
static int vfs_chmod(const char *file, mode_t mode) {
    USE(file);
    USE(mode);
    return 0;
}

/*
 * This function will change the user and group of the file
 * to be uid and gid.  This should only update the file's owner
 * and group.
 */
static int vfs_chown(const char *file, uid_t uid, gid_t gid) {
    USE(file);
    USE(uid);
    USE(gid);
    return 0;
}

/*
 * This function will update the file's last accessed time to
 * be ts[0] and will update the file's last modified time to be ts[1].
 */
static int vfs_utimens(const char *file, const struct timespec ts[2]) {
    USE(file);
    USE(ts);
    return 0;
}

/*
 * This function will truncate the file at the given offset
 * (essentially, it should shorten the file to only be offset
 * bytes long).
 */
static int vfs_truncate(const char *file, off_t offset) {
    USE(file);
    USE(offset);
  /* 3600: NOTE THAT ANY BLOCKS FREED BY THIS OPERATION SHOULD
           BE AVAILABLE FOR OTHER FILES TO USE. */

    return 0;
}

/* Call dread with the block index in the given block pointer and the given
  block, returns the value returned by dread */
int dread2(blocknum pointer, char* block) {
  int result = dread(pointer.block, block);
  if (result < 0) {
    printf("Error: Issue when attempting to write");
  }
  return result;
}

/* Call dwrite with the block index in the given block pointer and the given
  block, returns the value returned by dwrite */
int dwrite2(blocknum pointer, char* block) {
  int result = dwrite(pointer.block, block);
  if (result < 0) {
    printf("Error: Issue when attempting to write");
  }
  return result
}

/* Search the cache tree to find the given block number. If their is no vertex
  with the given block number then create a new vertex with than bnum with a 
  block pointing to NULL. */
vertex* search_cache(blocknum bnum) {
  vertex* vtx = cache.root;
  while (true) {
    blocknum bnum1 = vtx->disk_index;
    if (equals_blocknum(bnum == vtx->disk_index)) {
      return vtx;
    } else if ((unsigned int)bnum < (unsigned int)bnum1) {
      if (vtx->leftchild == NULL) {
        return add_vertex(vtx, LEFT, bnum, NULL);
      } else {
        vtx = vtx->leftchild;
      }
    } else {
      if (vtx->leftchild == NULL) {
        return add_vertex(vtx, RIGHT, bnum, NULL);
      } else {
        vtx = vtx->rightchild;
      }
    }
  }
}

/* Add the a new vertex with the given block number and block as the child 
  coresponding to the given child_num of the given parent vertex. Add this 
  vertex to the end of the priority queue and remove a vertex if the queue
  is full. Also add the vertex to Red-Black Tree and balance the tree. Returns
  the new vertex. The vertex is allocated but the char* must be given 
  allocated. */
vertex* add_vertex(vertex* parent, int child_num, blocknum bnum, char* block) {
  make_room_in_cache();
  cache.num_iter_active++;
  vertex* newvtx = (vertex*) calloc(sizeof(vertex), 1);
  vertex* oldtail = cache.tail;
  if (cache.head == NULL) {
    cache.head = newvtx;
  }
  if (oldtail != NULL) {
    oldtail->next = newvtx;
  }
  cache.tail = newvtx;
  *newvtx = {RED, child_num, parent, NULL, NULL, 0, oldtail, NULL, bnum, block};
  // TODO: Balance untested
  // balance(newvtx);
  return newvtx;
}

/* Balance the Red-Black Tree starting at the given vertex */
void balance(vertex* vtx) {
  for(;vtx != NULL; vtx = vtx->parent) {
    if (vtx.color == BLACK) {
      vertex* left = vtx->leftchild;
      if (left != NULL && left->color == RED) {
        vertex* leftleft = left->leftchild;
        if (leftleft != NULL && leftleft.color == RED) {
          vtx = balancell(vtx, left, leftleft);
          continue;
        }
        vertex* leftright = left->rightchild;
        if (leftright != NULL && leftright.color == RED) {
          vtx = balancelr(vtx, left, leftright);
          continue;
        }
      }
      vertex* right = vtx->rightchild;
      if (right != NULL && right->color == RED) {
        vertex* rightleft = right->leftchild;
        if (rightleft != NULL && rightleft.color == RED) {
          vtx = balancerl(vtx, right, rightleft);
          continue;
        }
        vertex* rightright = right->rightchild;
        if (rightright != NULL && rightleft.color == RED) {
          vtx = balancerr(vtx, right, rightleft);
          continue;
        }
      }
    }
  }
}

/* Balance the given left left imbalance. The central vertex is the black.
  whose left child is the vertex left and whose left-left grandchild is the 
  vertex leftleft */
vertex* balancell(vertex* central, vertex* left, vertex* leftleft) {
  vertex* llbranch = leftleft->leftchild;
  vertex* lrbranch = leftleft->rightchild;
  vertex* rlbranch = left->rightchild;
  vertex* rrbranch = central->rightchild;
  set_child(central->parent, central->child_num, left);
  set_children(left, leftleft, central);
  set_children(leftleft, llbranch, lrbranch);
  set_children(central, rlbranch, rrbranch);
  return left;
}

/* Balance the given left right imbalance. The central node is the black vertex.
  whose left child is the vertex left and whose left-right grandchild is the 
  vertex leftright */
vertex* balancelr(vertex* central, vertex* left, vertex* leftright) {
  vertex* llbranch = left->leftchild;
  vertex* lrbranch = leftright->leftchild;
  vertex* rlbranch = leftright->rightchild;
  vertex* rrbranch = central->rightchild;
  set_child(central->parent, central->child_num, leftright);
  set_children(leftright, left, central);
  set_children(left, llbranch, lrbranch);
  set_children(central, rlbranch, rrbranch);
  return leftright;
}

/* Balance the given right left imbalance. The central node is the black vertex.
  whose right child is the vertex right and whose right-left grandchild is the 
  vertex rightleft */
vertex* balancerl(vertex* central, vertex* right, vertex* rightleft) {
  vertex* llbranch = central->leftchild;
  vertex* lrbranch = rightleft->leftchild;
  vertex* rlbranch = rightleft->rightchild;
  vertex* rrbranch = right->rightchild;
  set_child(central->parent, central->child_num, rightleft);
  set_children(rightleft, central, right);
  set_children(central, llbranch, lrbranch);
  set_children(right, rlbranch, rrbranch);
  return rightleft;
}

/* Balance the given right right imbalance. The central node is the black 
    vertex. Whose right child is the vertex right and whose right-right 
    grandchild is the vertex rightright */
vertex* balancerr(vertex* central, vertex* right, vertex* rightright) {
  vertex* llbranch = central->leftchild;
  vertex* lrbranch = right->leftchild;
  vertex* rlbranch = rightright->leftchild;
  vertex* rrbranch = rightright->rightchild;
  set_child(central->parent, central->child_num, right);
  set_children(right, central, rightleft);
  set_children(central, llbranch, lrbranch);
  set_children(rightleft, rlbranch, rrbranch);
  return right;
}

/* Set the given child vertex as the child according with the given child number
    of the given parent. Unless the parent is NULL then do nothing */
void set_child(vertex* parent, unsigned int childn, vertex* child) {
  if (parent == NULL) {
    return;
  } else if (childn == LEFT) {
    parent->leftchild = child;
    child->child_num = LEFT;
    child->parent = parent;
  } else {
    parent->rightchild = child;
    child->child_num = RIGHT;
    child->parent;
  }
}

/* Set the children of the given parent to the given right vertex and the given
    left vertex */
void set_children(vertex* parent, vertex* left, vertex* right) {
  left->child_num = LEFT;
  right->child_num = RIGHT;
  parent->leftchild = left;
  parent->rightchild = right;
  left->parent = parent;
  right->parent = parent;
}

/* Make room in the cache for a new element to be removed if the cache is 
  full. Return 0 xor if dwrite is called then return what dwrite returns.
 */
int make_room_in_cache(){
  if (cache.num_elem >= cache.max_num_elem) {
    return remove_vertex(cache.head);
  }
  return 0;
}

/* Remove the given vertex form the cache in both the Red-Black Tree and
  Priority queue. If the vertex is NULL then nothing should change otherwise
  if the vertex is the root set the new root and if it is the head, 
  set the new head of the priority queue. */
int remove_vertex(vertex* vtx) {
  return 0;/*
  if (vtx == NULL || cache.iter->singlenum > 0 ||
   (vtx->color == BLACK && !vtx->disk_index.valid)) {
    return 0;
  } else {
    // Remove the vertex from the priority queue
    if (vtx->prev == NULL && vtx->next == NULL) {
      cache.head = NULL;
      cache.tail = NULL;
    } else if (vtx->prev == NULL) {
      cache.head = vtx->next;
      vtx->next->prev = NULL;
    } else if (vtx->next == NULL) {
      cache.tail = vtx->prev;
      vtx->prev->next = NULL;
    } else {
      vertex* next_ver = vtx->next;
      vertex* prev_ver = vtx->prev;
      next_ver->prev = prev_ver;
      prev_ver->next = next_ver;
    }
    // Write the block of the vertex if necessary
    int dwriteresult = 0;
    if (vtx->writes > 0) {
      dwriteresult = dwrite2(vtx->disk_index, vtx->block);
      vtx->writes = 0;
    }
    // Free the block
    free(vtx->block);
    // Remove the vertex from the Red-Black Tree
    if (vtx->color == RED) {
      if (vtx->rightchild == NULL && vtx->leftchild == NULL) {
        remove_red_no_child(vtx);
      } else if (vtx->rightchild == NULL) {
        remove_red_left_child(vtx);
      } else if (vtx->leftchild == NULL) {
        remove_red_right_child(vtx);
      } else {
        remove_red_two_children(vtx);
      }
    } else {
      if (vtx->rightchild == NULL && vtx->leftchild == NULL) {
        remove_black_no_child(vtx);
      } else if (vtx->rightchild == NULL) {
        if (vtx->leftchild.color == RED) {
          remove_black_right_red_child(vtx);
        } else {
          remove_black_right_black_child(vtx);
        }
      } else if (vtx->rightchild == NULL) {
        if (vtx->leftchild.color == RED) {
          remove_black_left_red_child(vtx);
        } else {
          remove_black_left_black_child(vtx);
        }
      } else {
        if (vtx->leftchild.color == RED) {
          if (vtx->rightchild.colo == RED) {
            remove_black_two_red_children(vtx);
          } else {
            remove_black_left_red_right_black(vtx);
          }
        } else {
          if (vtx->rightchild.color == RED) {
            remove_black_left_black_right_red(vtx);
          } else {
            remove_black_two_black_children(vtx);
          }
        }
      } 
    }
    return dwriteresult;
  }*/
}

/* Remove the given RED vertex with no children */
void remove_red_no_child(vertex* vtx) {
    
}

/* Remove the given RED vertex with only a right child */
void remove_red_right_child(vertex* vtx) {
    
}

/* Remove the given RED vertex with only a left child */
void remove_red_left_child(vertex* vtx) {
    
}

/* Remove the given RED vertex with two children */
void remove_red_two_children(vertex* vtx) {
    
}

/* Remove the given BLACK vertex with no children */
void remove_black_no_child(vertex* vtx) {
    
}

/* Remove the given BLACK vertex with a only right RED child */
void remove_black_right_red_child(vertex* vtx) {
    
}

/* Remove the given BLACK vertex with a only right BLACK child */
void remove_black_right_black_child(vertex* vtx) {
    
}

/* Remove the given BLACK vertex with a only left RED child */
void remove_black_left_red_child(vertex* vtx) {
    
}

/* Remove the given BLACK vertex with a only left BLACK child */
void remove_black_left_black_child(vertex* vtx) {
    
}

/* Remove the given BLACK vertex with a right RED child & a left BLACK child */
void remove_black_left_black_right_red(vertex* vtx) {
    
}

/* Remove the given BLACK vertex with a left RED child & a right BLACK child */
void remove_black_left_red_right_black(vertex* vtx) {
    
}

/* Remove the given BLACK vertex with two RED children */
void remove_black_two_red_children(vtx) {
    
}

/* Remove the given BLACK vertex with two BLACK children */
void remove_black_two_black_children(vtx) {
    
}

/* Return the vertex with the maximum block number in the given tree. The given
  vertex cannot be NULL. */
vertex* get_tree_max(vertex* vtx) {
  for (; vtx->rightchild == NULL; vtx = vtx->rightchild) {
    // Do nothing
  }
  return vtx;
}

/* Return the vertex with the maximum block number in the given tree. The given
  vertex cannot be NULL. */
vertex* get_tree_min(vertex* vtx) {
  for (; vtx->leftchild == NULL; vtx = vtx->leftchild) {
    // Do nothing
  }
  return vtx;
}

/* Compares two blocknums for equality. Returns true if they are structurally
  equal, false otherwise */
bool equals_blocknum(blocknum b0, blocknum b1) {
  return b0.block == b1.block && b0.valid == b1.valid;
}

/* Add the given dirent at the given block nummber into the given dnode.
  Return 0 if the function ran without a problem. -1 if the DNODE is already full.
  -2 if a file of that name already exists within the DNODE. Return -3 for all
  errors generated by dread of dwrite. -4 if no more room is availible in the free list. */
int add_to_dnode(dnode* dir, blocknum drntloc, dirent* drnt) {
  unsigned int hash = hashstr(drnt->name);
  hash_bnum new_elem = {hash, drntloc};
  hash_bnum* iter = make_iterator_dnode(dir);
  while (hasnext_diter(iter)) {
    hash_bnum hb = next_diter(iter);
    if (!hb.block.valid) {
      return -3;
    } else if (bh.hash == hash) {
      if (equals_blocknum(drntloc, bh.block)) {
        return -2;
      } else {
        dirent* drnt1 = (dirent*) read_blocknum(bh.block);
        if (drnt1 == NULL) {
          return -3;
        } else if (strcmp(drnt->name, drnt1->name) == 0) {
          return -2;
        }
      }
    }
  }
  int adanl_result = add_dirent_at_next_loc(iter, drntloc);
  remove_dnode_iterator(iter);
  return adanl_result < 0 ? -3 : 0;
}

/* Add the given dirent location in the next location in the given iterator.
  The given iterator MUST not have any elements remaining. */
int add_dirent_at_next_loc(dnode_iter* iter, blocknum drntloc) {
  if (iter->singlenum == -2) {
    return add_drnt_nxt_loc_directs(iter, drntloc);
  } else if (iter->singlenum == -1) {
    return add_drnt_nxt_loc_single(iter, drntloc);
  } else if (iter->doublenum == -1) {
    return add_drnt_nxt_loc_double(iter, drntloc);
  } else {
    return add_drnt_nxt_loc_trip(iter, drntloc);
  }
  iter->dir->size++;
}

/* Add the given dirent int the next location in the iterator given that
  the iterator ended within the directs */
int add_drnt_nxt_loc_directs(dnode_iter* iter, blocknum drntloc) {
  // If the iterator ends at the last element in the directs
  // Make the first indirect and 
  if (iter->index >= NUM_OF_DRNTS) {
    blocknum bnum0 = pop_next_free();
    iter->dir->single_indirect = bnum0;
    if (!bnum0.valid) {
      return -1;
    }
    hash_indirect* hashind = create_empty_hash_ind(bnum0);
    hashind->block[0] = drntloc;
    return write_blocknum(bnum0, (char*)hashind);
  }
  iter->dir->directs[iter->index] = drntloc;
  return 0;
}

/* Add the given dirent int the next location in the iterator given that
  the iterator ended within the single indirect */
int add_drnt_nxt_loc_single(dnode_iter* iter, blocknum drntloc) {
  // If the iterator ends at the last element in the first redirect.
  // Make the double indirect and the single indirect in the 0th position
  // in that double indirect.
  if (iter->index >= NUM_IN_HASH_IND) {
    blocknum bnum0 = pop_next_free();
    iter->dir->double_indirect = bnum0;
    if (!bnum0.valid) {
      return -1;
    }
    indirect* ind = create_empty_ind(bnum0);
    blocknum bnum1 = pop_next_free();
    iter->dir->single_indirect->block[0] = bnum1;
    if (!bnum1.valid) {
      return -1;
    }
    hash_indirect* hashind = create_empty_hash_ind(bnum1);
    ind->blocks[0] = bnum1;
    int result = write_blocknum(bnum0, (char*)ind);
    hashind->blocks[0] = drntloc;
    return result + write_blocknum(bnum1, (char*)hashind);
  }
  iter->single_ind->blocks[iter->index] = drntloc;
  return write_blocknum(iter->single_bn, (char*)iter->single_ind);
}

/* Add the given dirent int the next location in the iterator given that
  the iterator ended within the double indirect. May have bug.*/
int add_drnt_nxt_loc_double(dnode_iter* iter, blocknum drntloc) {
  // If the iterator ends at the last element in the first redirect.
  // Make the double indirect and the single indirect in the 0th position
  // in that double indirect.
  if (iter->index >= NUM_IN_HASH_IND) {
    int result = 0;
    blocknum bnum0 = pop_next_free();
    if (!bnum0.valid) {
      return -1;
    }
    hash_indirect* hashind = create_empty_hash_ind(bnum0);
    hashing->blocks[0] = drntloc;
    result += write_blocknum(bnum0, (char*)hashind);
    if (iter->singlenum >= NUM_IN_IND) {
      blocknum bnum1 = pop_next_free();
      if (!bnum1.valid) {
        return -1;
      }
      indirect* doub = create_empty_ind(bnum1);
      blocknum bnum2 = pop_next_free();
      if (!bnum2.valid) {
        return -1;
      }
      indirect* trip = create_empty_ind(bnum2);
      trip->blocks[0] = bnum1;
      result += write_blocknum(dnum2, (char*)trip);
      doub->blocks[0] = bnum0;
      result += write_blocknum(dnum1, (char*) doub);
    } else {
      indirect* ind = iter->dir->double_ind;
      ind->blocks[singlenum] = bnum0;
      result += write_blocknum(iter->dir->double_indirect, ind);
    }
    return result;
  }
  iter->single_ind->blocks[iter->index] = drntloc;
  return write_blocknum(iter->single_bn, (char*)iter->single_ind);
}

/* Add the given dirent int the next location in the iterator given that
  the iterator ended within the triple indirect May have bug. */
int add_drnt_nxt_loc_trip(dnode_iter* iter, blocknum drntloc) {
  if (iter->index >= NUM_IN_HASH_IND) {
    int result = 0;
    locknum bnum0 = pop_next_free();
    if (!bnum0.valid) {
      return -1;
    }
    hash_indirect* hashind = create_empty_hash_ind(bnum0);
    hashing->blocks[0] = drntloc;
    result += write_blocknum(bnum0, (char*)hashind);
    if (iter->singlenum >= NUM_IN_IND) {
      blocknum bnum1 = pop_next_free();
      if (!bnum1.valid) {
        return -1;
      }
      indirect* doub = create_empty_ind(bnum1);
      doub->blocks[0] = bnum0;
      result += write_blocknum(dnum1, (char*) doub);
      iter->trip_ind->blocks[doublenum] = bnum1;
      result += write_blocknum(iter->trip_bn, iter->trip_ind);
    } else {
      indirect* ind = iter->dir->double_ind;
      ind->blocks[singlenum] = bnum0;
      result += write_blocknum(iter->double_bn, ind);
    }
    return result;
  }
  iter->single->blocks[iter->index] = drntloc;
  return write_blocknum(iter->single_bn, (char*)iter->single_ind);
}

/* Make an iterator over the given dirents of the given dnode, dir, that cannot
  be NULL. The iterator will be as lond as the size of dnode. While their is a
  dnode being run, the cache will not be able to remove elements. */
dnode_iter* make_dnode_iterator(dnode* dir) {
  dnode_iter* result = (dnode_iter*) calloc(dnode_iter, 1);
  *result = {0, -2, -2, 0, 0, NULL, NULL, INV_BNUM, NULL};
  cache.num_iter_active++;
  return result;
}

/* Free the given iterator and subtract one from the active iterators over data
  within the cache */
void remove_dnode_iterator(dnode_iter* iter) {
  cache.num_iter_active--;
  free(iter);
  if (cache.num_iter_active <= 0 && cache.num_elem > cache.max_num_elem) {
    make_room_in_cache();
  }
}

/* Get the next element from the given iterator. The function will have an error
  if not surronded by the hasnext_diter. Return HASH_INV_BNUM if an error 
  occured */
hash_bnum next_diter(dnode_iter* iter) {
  if (iter->singlenum == -2) {
    return next_diter_direct(iter);
  } else if (iter->singlenum == -1) {
    return next_diter_single(iter)
  } else if (iter->doublenum == -1) {
    return next_diter_double(iter);
  } else {
    return next_diter_trip(iter);
  }
}

/* Get the next element from the given iterator while the iterator is within the
  directs. The function will have an error if not surronded by the 
  hasnext_diter. Return HASH_INV_BNUM if an error occured */
hash_bnum next_diter_direct(dnode_iter* iter) {
  // The iterator is currently within directs within the directory.
  if (iter->index >= NUM_OF_DRNTS) {
    // There are no more directs then move into the first indirect is it exists
    // and it does not cause a read error.
    // Set the location values of the iterator to {-2, -2, -1, 0} which means 
    // the next element is the 0th position in the single indirect.
    iter->singlenum = -1;
    iter->index = 0;
    iter->single_bn = iter->dir->single_indirect;
    // If the block number of the single indirect is valid then it exist and
    // should be moved into
    if (iter->single_bn.valid) {
      iter->single_ind = (hash_indirect*) read_blocknum(iter->single_bn);
      if (iter->single_ind == NULL) {
        return HASH_INV_BNUM;
      }
      // Return the next iteration
      return next_diter(iter);
    } else {
      // The block does not yet exist
      iter->single_bn = NULL;
      return HASH_INV_BNUM;
    }
  } else {
    // Normally iterate through the directs
    iter->num_iter++;
    return iter->dir->direct[iter->index++];
  }
}

/* Get the next element from the given iterator while the iterator is within the
  single indirect. The function will have an error if not surronded by the 
  hasnext_diter. Return HASH_INV_BNUM if an error occured */
hash_bnum next_diter_single(dnode_iter* iter) {
  // If the iterator is within the single indirect then this area is reached
  if (iter->index >= NUM_IN_HASH_IND) {
    // The iterator should move into the double indirect if it exists and it
    // does not cause a read error.
    // Set the location values of the iterator to {-2, -1, 0, 0} which means the
    // next element is the 0th position in the 0th single indirect in the 
    // directory's double indirect.
    iter->doublenum = -1;
    iter->singlenum = 0;
    iter->index = 0;
    iter->double_bn = iter->dir->double_indirect;
    if (iter->double_bn.valid) {
      // If the current double block num is valid then move into it
      iter->double_ind = (indirect*) read_blocknum();
      if (iter->double_ind == NULL) {
        // Check whether the read produced an error
        iter->single_bn = INV_BNUM;
        iter->single_ind = NULL;
        return HASH_INV_BNUM;
      }
      iter->single_bn = iter->double_ind->blocks[0];
      if (iter->single_bn.valid) {
        // If the current single block num is valid then move into it
        iter->single_ind = (hash_indirect*) read_blocknum(iter->single_bn);
        if (iter->single_ind == NULL) {
          return HASH_INV_BNUM;
        }
        // Return the next iteration
        return next_diter(iter);
      } else {
        // The single block does not exist
        iter->single_ind = NULL;
        return HASH_INV_BNUM;
      }
    } else {
      // The double block does not exist thus the first single indirect within
      // it does not exist either.
      iter->double_ind = NULL;
      iter->single_bn = INV_BNUM;
      iter->single_ind = NULL;
      return HASH_INV_BNUM;
    }
    
    return next_diter(iter);
  } else {
    // Get the next element in iterator in the single indirect
    iter->num_iter++;
    return iter->single_ind->blocks[iter->index++];
  }
}

/* Get the next element from the given iterator while the iterator is within the
  double indirect. The function will have an error if not surronded by the 
  hasnext_diter. Return HASH_INV_BNUM if an error occured */
hash_bnum next_diter_double(dnode_iter* iter) {
  // If the iterate is within the double indirect then this area is reached
  if (iter->index >= NUM_IN_HASH_IND) {
    // The iterator moves to the next single indirect in the second indirect
    // or move into the third indirect.
    if (iter->singlenum >= NUM_IN_IND - 1) {
      // If done iterating through the double indirect move into the triple
      // indirect. Set the new location of the iterator to {1, 0, 0, 0}.
      iter->tripnum = 1;
      iter->doublenum = 0;
      iter->singlenum = 0;
      iter->index = 0;
      iter->trip_bn = iter->dir->trip_indirect;
      // Check whether the triple indirect exist. If it doesn't don't continue
      if (iter->trip_bn.valid) {
        // Set the indirect to the read from the block
        iter->trip_ind = (indirect*) read_blocknum(iter->trip_bn);
        if (iter->trip_ind == NULL) {
          // A error occured during the read so set the values to invalid and NULL values
          iter->double_bn = INV_BNUM;
          iter->double_ind = NULL;
          iter->single_bn = INV_BNUM;
          iter->single_ind = NULL;
          return HASH_INV_BNUM;
        }
        // Set the double block number to the 0th position of the triple indirect
        iter->double_bn = iter->trip_ind->blocks[0];
        // Check whether the 0th double indirect exists
        if (iter->double_bn.valid) {
          iter->double_ind = (indirect*) read_blocknum(iter->double_bn);
          if (iter->double_ind == NULL) {
            // An error has occured in the read
            iter->single_bn = INV_BNUM;
            iter->single_ind = NULL;
            return HASH_INV_BNUM;
          }
          // Set the single block number to the 0th position of the double indirect
          iter->single_bn = iter->double_ind->blocks[0];
          // Check whether the 0th single indirect exists
          if (iter->single_bn.valid) {
            iter->single_ind = (hash_indirect*) read_blocknum(iter->single_bn);
            if (iter->single_ind == NULL) {
              // An error has occured in the read
              return HASH_INV_BNUM;
            }
            // Move to the next element with dnode
            return next_diter(iter);
          } else {
            // The single indirect does not exist
            iter->single_ind = NULL;
            return HASH_INV_BNUM;
          }
        } else {
          // The second double indirect does not exist thus the single indirect does
          // not exist either.
          iter->double_ind = NULL;
          iter->single_bn = INV_BNUM;
          iter->single_ind = NULL;
          return HASH_INV_BNUM;
        }
      } else {
        // The trip block does not exist thus the first double and single indirect within
        // it do not exist either.
        iter->trip_ind = NULL;
        iter->double_bn = INV_BNUM;
        iter->double_ind = NULL;
        iter->single_bn = INV_BNUM;
        iter->single_ind = NULL;
        return HASH_INV_BNUM;
      }
    } else {
      // Go to the 0th element in the next single indirect within the double indirect
      iter->index = 0;
      iter->single_bn = iter->double_ind->blocks[++iter->singlenum];
      // Check whether the next single indirect exists
      if (iter->single_bn.valid) {
        iter->single_ind = (hash_indirect*) read_blocknum(iter->single_bn);
        if (iter->single_ind == NULL) {
          return HASH_INV_BNUM;
        }
        // Move to next element in the dnode
        return next_diter(iter);
      } else {
        // The next single indirect doesn't exist
        iter->single_ind = NULL;
        return HASH_INV_BNUM;
      }
    }
  } else {
    // Get the next element in iterator in the single indirect
    iter->num_iter++;
    return iter->single_ind->blocks[iter->index++];
  }
}

/* Get the next element from the given iterator while the iterator is within the
  triple indirect. The function will have an error if not surronded by the 
  hasnext_diter. Return HASH_INV_BNUM if an error occured */
hash_bnum next_diter_trip(dnode_iter* iter) {
  // If the iterate is within the triple indirect then this area is reached
  if (iter->index >= NUM_IN_HASH_IND) {
    // The iterator moves to the next single indirect in the second indirect
    // and into the next second indirect if necessary
    if (iter->singlenum >= NUM_IN_IND - 1) {
      if (iter->doublenum >= NUM_IN_IND - 1) {
        iter->tripnum = 2;
        return HASH_INV_BNUM;
      } else {
        // Go to the 0th index within the next single indirect within the double indirect
        iter->index = 0;
        iter->double_bn = iter->trip_ind->blocks[++iter->doublenum];
        // Check whether the double indirect block exist
        if (iter->double_bn.valid) {
          iter->double_ind = (indirect*) read_blocknum(iter->double_bn);
          if (iter->double_ind == NULL) {
            // An error has occured
            iter->single_bn = HASH_INV_BNUM;
            iter->single_ind = NULL;
            return HASH_INV_BNUM;
          }
          // Set the single block number to the 0th within second indirect
          iter->single_bn = iter->double_ind->blocks[0];
          // Check whether the single indirect block number is valid
          if (iter->single_bn.valid) {
            iter->single_ind = (hash_indirect*) read_blocknum(iter->single_bn);
            if (iter->single_ind == NULL) {
              // A read error has occured
              return HASH_INV_BNUM;
            }
            return next_diter(iter);
          } else {
            // The single indirect does not exist
            iter->single_ind = NULL;
            return HASH_INV_BNUM;
          }
        } else {
          // The double block indirect block exist thus the 0th element within it does not exist
          iter->double_ind = NULL;
          iter->single_bn = HASH_INV_BNUM;
          iter->single_ind = NULL;
          return HASH_INV_BNUM;
        }
      }
    } else {
      // Go to the 0th element in the next single indirect within the double indirect
      iter->index = 0;
      iter->single_bn = iter->double_ind->blocks[++iter->singlenum];
      // Check whether single indirect was valid
      if (iter->single_bn.valid) {
        iter->single_ind = (hash_indirect*) read_blocknum(iter->single_bn);
        if (iter->single_ind == NULL) {
          // Check for an error in the read blocknum
          return HASH_INV_BNUM;
        }
        return next_diter(iter);
      } else {
        // The single indirect does not exist
        iter->single_ind = NULL;
        return HASH_INV_BNUM;
      }
    }
  } else {
    // Get the next element in iterator in the triple indirect
    iter->num_iter++;
    return iter->single_ind->blocks[iter->index++];
  }
}

/* Does the given iterator have a next element? */
bool hasnext_diter(dnode_iter* iter) {
  return iter->num_iter >= iter->dir->size;
}

/* Hash the given null terminating cstring */
unsigned int hashstr(char* str) {
  unsigned int result = '\0';
  while (*str != '\0') {
    result += *(str++) * 11;
  }
  return result;
}

/* Read the block at the given blocknum in the file system. If it is in the cache, grab it
  from the cache. Overwise read from the hard drive and add it to the cache. Returns the block
  or NULL for errors */
char* read_blocknum(blocknum bnum) {
  vertex* vtx = search_cache(bnum);
  if (vtx == NULL) {
    char* block = (char*) calloc(sizeof(char), 512);
    vtx->block = block;
    if (dread2(bnum, block) < 0) {
      return NULL;
    }
    return block;
  } else {
    return vtx.block;
  }
}

/* Write the block at the given blocknum in the file system. If it is in the cache, write to
  the value in the cache and after a number of writes, write to the drive. If not within cache,
  create a new value in the cache for the block. Return the value dwrite would return if called. */
int write_blocknum(blocknum bnum, char* block) {
  int result = 0;
  vertex* vtx = search_cache(bnum);
  vtx->writes++;
  if (vtx == NULL) {
    vtx->block = block;
  } else {
    free(vtx->block);
    vtx->block = block;
  }
  if (vtx->writes >= 4) {
    result = dwrite2(bnum, block);
    vtx->writes = 0;
  }
  return result;
}

/* Create an empty hash indirect with only invalid hash-blocknums */
hash_indirect* create_empty_hash_ind(blocknum bnum) {
  hash_indirect* result = (hash_indirect*)read_blocknum(bnum);
  for (int i = 0; i < NUM_IN_HASH_IND; i++) {
    ind->blocks[i] = HASH_INV_BNUM;
  }
  return result;
}

/* Create a empty indirect with only invalid blocknums */
indirect* create_empty_ind(blocknum bnum) {
  indirect* result = (indirect*)read_blocknum(bnum);
  for (int i = 0; i < NUM_IN_IND; i++) {
    ind->blocks[i] = INV_BNUM;
  }
  return result;
}

/* Create a new dnode at the given location, loc, with the given parent dnode,
  with the given parent block number, p_loc, with the given mode. If the parent
  is NULL then this node has no parent which makes .. point to itself. Does 
  no writes. */
dnode* new_dnode(blocknum loc, blocknum p_loc, dnode* parent, mode_t mode) {
  hash_bnum dirents[53];
  for (int i = 0; i < NUM_OF_DRNTS; i++) {
    dirents[i] = HASH_INV_BNUM;
  }
  struct timespec currenttime = clock_gettime(CLOCK_REALTIME, 
    &volcb.access_time);
  dnode newnode = {0, 1, getuid(), getgid(), mode, currenttime, currenttime, 
    currenttime, dirents, INV_BNUM, INV_BNUM, INV_BNUM};
  add_new_hardlink(lec, &newnode, ".", loc, &newnode);
  if (parent == NULL) {
    add_new_hardlink(loc, &newnode, "..", loc, &newnode);
  } else {
    add_new_hardlink(loc, &newnode, "..", p_loc, parent);
  }
  return newnode;
}

/* Add the a new hardlink in the given dnode, dir, with the given name
  and block number to the given dnode destination, dest. Writes only the new
  dirent to the hard drive but does. */
void add_new_hardlink(block dir_bn, dnode* dir, char* name, blocknum bn_dest, 
  dnode* dest) {
  blocknum drntbnum = pop_next_free();
  if (!drntbnum.valid) {
    return;
  }
  dest->links++;
  dirent drnt = {name, 'd', bn_dest};
  write_blocknum(drntbnum, (char*)drnt);
  add_to_dnode(dir, drntbnum, drnt)
}

/* Given a cstring path, return a NULL-terminated array of cstring tokens of
  each part of the path in order. */
char** parse_path(char* path){
  char** toks = (char**) calloc(20, sizeof(char*));
  bool intok = false;
  // position in path
  unsigned int index = 0;
  // position in token array
  unsigned int tokindex = 0;
  // length of the current token
  unsigned int toklen = 0;
  while (path[index] != '\0') {
    if (intok == false && toklen > 0) {
      char* tok = (char*) calloc(toklen + 1, sizeof(char));
      strncpy(path+index-toklen-1, tok, toklen);
      tok[toklen] = '\0';
      toks[tokindex++] = tok;
      toklen = 0;
    } else if (intok == false) {
      intok = true;
    } else if (intok && path[index] == '/') {
      index++;
      intok = false;

    } else {
      toklen++;
      index++;
    }
    toks[tokindex] = NULL;
  }
  return toks;
}

/* Get the dirent that points to the file or directory by the given path.
    Returns NULL if the given file or directory path doesn't actually lead to a
    file or directory. */
dirent* get_from_path(char* path) {
  dirent* ent = (dirent*) calloc(1, sizeof(dirent));
  char** toks = parse_path(path);
  // Number of tokens in toks
  int tokslen = 0;
  while (toks[tokslen] != NULL)
    tokslen++;
  // How many directories deep are we
  int curdepth = 0;
  // Find location in root
  dnode_iter* root_iter = make_dnode_iterator(&root);
  while (hasnext_diter(root_iter)) {
    hash_bnum next = next_diter(root_iter);
    dirent* next_block = (dirent*) read_blocknum(next.bnum);
    if (next_block != NULL && strcmp(next_block->name, toks[curdepth]) = 0) {
      remove_dnode_iterator(root_iter);
      cur_depth++;
      if (cur_depth == tokslen) {
        return next_block;
      } else if (/* START HERE, CODY */) {
      }
    }
  }
}

make - dnode_iter* make_dnode_iterator(dnode*);
next - hash_bnum next_diter(dnode_iter*); // Returns invalid bnum if error occurred
hasnext - bool hasnext_diter(dnode_iter*); // Should precede all next calls
free - void remove_dnode_iterator(dnode_iter*); 

/*
 * You shouldn't mess with this; it sets up FUSE
 *
 * NOTE: If you're supporting multiple directories for extra credit,
 * you should add 
 *
 *     .mkdir  = vfs_mkdir,
 */
static struct fuse_operations vfs_oper = {
    .init    = vfs_mount,
    .destroy = vfs_unmount,
    .getattr = vfs_getattr,
    .readdir = vfs_readdir,
    .create  = vfs_create,
    .read  = vfs_read,
    .write   = vfs_write,
    .unlink  = vfs_delete,
    .rename  = vfs_rename,
    .chmod   = vfs_chmod,
    .chown   = vfs_chown,
    .utimens   = vfs_utimens,
    .truncate  = vfs_truncate,
    .mkdir = vfs_mkdir,
};

int main(int argc, char *argv[]) {
    /* Do not modify this function */
    umask(0);
    if ((argc < 4) || (strcmp("-s", argv[1])) || (strcmp("-d", argv[2]))) {
      printf("Usage: ./3600fs -s -d <dir>\n");
      exit(-1);
    }
    return fuse_main(argc, argv, &vfs_oper, NULL);
}
