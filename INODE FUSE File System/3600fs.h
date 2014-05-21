/*
 * CS3600, Spring 2014
 * Project 2 Starter Code
 * (c) 2013 Alan Mislove
 *
 */

#ifndef __3600FS_H__
#define __3600FS_H__
#include <unistd.h>
#include <time.h>
#include <fuse.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>
#include <sys/statfs.h>
#include "disk.h"

/* Size in bytes of each block */
//const unsigned int BLOCKSIZE = 512;

/* Our magic number */
const int MAGICNUM = -1836311903;

/* Boolean typedef (because we like our lowercase boolean keywords) */
typedef unsigned int bool;
enum { false, true };

/* The constant for left child number */
unsigned int LEFT = 0;

/* The constant for right child number */
unsigned int RIGHT = 1;

/* The constant for red color */
unsigned int RED = 0;

/* The constant for black color */
unsigned int BLACK = 1;

/* Block Pointer Structure:
		Represents a pointer to a block in the partition and a flag that states if
		the pointer is valid */
typedef struct {
	// Number of the block being pointed to
	unsigned int block:31;
	// Validity flag
	unsigned int valid:1;
} blocknum;

/* Hashcode and corresponding blocknum */
typedef struct {
	/* The hash of the file name */
	unsigned int hash;
	/* The blocknum corresponding to the hash */
	blocknum bnum;
} hash_bnum;

/* Blocknum constant of the VCB block. */
const blocknum VCB_BLOCKNUM = {0, true};

/* Blocknum constant of the root directory's DNODE block. */
const blocknum ROOT_BLOCKNUM = {1, true};

/* Volume Control Block Structure:
		Contains file system metadata */
typedef struct {
	// Magic (identification) number, used to determine if we made the partition
	int magic;
	// Number of blocks that the partition is
	int blocksize;
	// Block pointer to the root directory dnode
	blocknum root;
	// Block pointer to the first free block
	blocknum nextfree;
	// Block pointer to the last free block
	blocknum lastfree;
	// Name of this partition, pads the vcb to the size of the block
	char name[492];
} vcb;

/* The number of DIRENTS in the DNODE array of DIRENTS */
const int NUM_OF_DRNTS = 53;

/* The standard invalid block number */
const int INV_BNUM = {0, 0};

/* The standard invalid block number */
const int HASH_INV_BNUM = {0, INV_BNUM};

/* Directory Node Block Structure:
		Stores metadata on a directory and links to all dirents and indirects that
		point to the contents of the directory */
typedef struct {
	// Number of entries in this directory
	unsigned int size;
	// Number of hard links to this directory
	unsigned int links;
	// User that owns this directory
	uid_t user;
	// Group that owns this directory
	gid_t group;
	// TODO
	mode_t mode;
	// Time which this directory was last accessed
	struct timespec access_time;
	// Time at which this directory was last modified
	struct timespec modify_time;
	// Time at which this directory was created
	struct timespec create_time;
	// Array of direct links to directory contents
	//	*Only used if there are few enough entries to fit within a single block
	hash_bnum direct[53];
	// Hash & Block pointer to a single indirect block of directory entries
	//	*Only used when there there are too many entries to fit within a single
	//	 dnode block, but too few to have nested indirect blocks
	blocknum single_indirect;
	// Hash & Block pointer to a nested indirect block of directory entires and
	// indirect blocks
	//	*Only used if there are too many entries to fit in a single indirect
	//	 block
	blocknum double_indirect;
	// Hash & Block pointer to a nested indirect block of directory entires and
	// indirect blocks
	//	*Only used if there are too many entries to fit in a single indirect
	//	 block
	blocknum trip_indirect;
} dnode;

/* The number of elements in a hash indirect */
const int NUM_IN_HASH_IND = 64;

/* Hash/Blocknum Indirect Block Structure:
		Stores multiple block pointers to a dirent and the hash of its name */
typedef struct {
	// All of the block pointers to dirents or indirects
	hash_bnum blocks[64];
} hash_indirect;

/* The number of elements in a normal indirect */
const int NUM_IN_IND = 128;

/* Indirect Block Structure:
		Stores multiple block pointers to either dirents or other indirects */
typedef struct {
	// All of the block pointers to dirents or indirects
	blocknum blocks[128];
} indirect;

/* Directory Entry Block Structure:
		Stores the name and type of an entry in a directory and the block pointer
		to the block where its meta block is stored (either an inode or dnode) */
typedef struct {
	// The name of the entry being pointed to (the filename or directory name)
	char name[507];
	// Character representing the type of the block being pointed to
	char type;
	// Block pointer to the metadata block for this entry
	blocknum meta;
} dirent;

/* File Inode Block:
		Stores the metadata on a file and links to all blocks that contain the
		files data in order using direct block pointers or indirect blocks */
typedef struct {
	// Size of the file in bytes
	unsigned int size;
	// Number of hard links to this directory
	unsigned int links;
	// User that owns this file
	uid_t user;
	// Group that owns this file
	gid_t group;
	// TODO
	mode_t mode;
	// Time which this file was last accessed
	struct timespec access_time;
	// Time at which this file was last modified
	struct timespec modify_time;
	// Time at which this file was created
	struct timespec create_time;
	// Array of direct links to file content blocks
	//	*Only used if there are few enough entries to fit within a single block
	blocknum direct[109];
	// Block pointer to a single indirect block of file contents
	//	*Only used when there there are too many entries to fit within a single
	//	 dnode block, but too few to have nested indirect blocks
	blocknum single_indirect;
	// Block pointer to a nested indirect block of file contents and
	// indirect blocks
	//	*Only used if there are too many entries to fit in a single indirect
	//	 block
	blocknum nested_indirect;
} inode;

/* Free Block Structure:
		Stores a block pointer to the next free block in the partition */
typedef struct {
	blocknum next;
	char junk[508];
} freeblock;

/* Data Block Strucuture:
		Block that stores file data and nothing else */
typedef struct {
	char data[512];
} data;

/* List Block Structure:
		A vertex in the cache red-black tree list
		A NULL as the parent means this is the root of the tree
		A NULL for a child means that child is a leaf
		A NULL in prev means this is the first element in the list
		A NULL in next means this is the last element in the list*/
/* An Empty Black has a invalid blcoknum */
typedef struct s_vertex{
	// The color of the vertex in the red-black tree
	unsigned int color:1;
	// 0 represents that this is the left child of the parent
	// 1 represents that this is the right child of the parent
	unsigned int child_num:1;
	// The parent in the red-black tree
	struct s_vertex* parent;
	// The left child in the red-black tree
	struct s_vertex* leftchild;
	// The right child in the red-black tree
	struct s_vertex* rightchild;
	// Number of times the block was written to without being written to the
	// disk
	unsigned int writes:30;
	// Pointer to the next element in the list
	struct listlink_s* next;
	// Pointer to the previous element in the list
	struct listlink_s* prev;
	// Block pointer to the block's location on the disk
	blocknum disk_index;
	// The block itself
	char* block;
} vertex;

/* A Red-Black Tree - Linked List Structure:
		The tree which sorts based on block numbers in assending order
		Invariance:
			The left child is strictly less than its parent.
			The right child is strictly greater than its parent.
			Every RED vertex must only have BLACK children.
			Every vertex is inserted as a red node.
			The root node is always black.
			Every EMPTYBLACK node must only have leaf children.
		When a Black 
		The linked-list is strictly in ordered in a priority queue.
			Every time a new element is added, it is placed on the tail 
				of the queue.
			If room needs to be made in the queue, the head is removed.
			Any time a search hits a vertex it swaps with the next vertex 
				if availible. */
typedef struct {
    // The number of iterator currently running over the elements that must 
    // exist within the cache
    unsigned int num_iter_active;
	// The maximum number of elements permitted in the structure;
	unsigned int max_num_elem;
	// The number of elements in this structure
	unsigned int num_elem;
	// The root of the tree
	vertex* root;
	// The head of the list
	vertex* head;
	// The tail of the list
	vertex* tail;
} rbtreelist;


// An Iterator Over a DNODE Structure 
typedef struct {
  // 0 means the iterator is not in the trip' indirect
  // 1 means the iterator is in the trip' indirect
  // 2 means the end of all possible  elements has been reached
  unsigned int tripnum:2;
  // -2 means the iterator is not in the double or trip' indirect
  // -1 means the iterator is within the double indirect
  // 0 - 127 means the iterator is within that index in the trip' indirect
  int doublenum:10;
  // -2 means the iterator is not within an indirect
  // -1 means the iterator is within the single indirect
  // 0 - 127 means the iterator is within that index in a double indirect
  int singlenum:10;
  // 0 - 63 is the location of the next dirent in either the dnode dirents array
  // or within the hash_indirect array
  int index:10;
  // The number of dirents that have been iterated through already
  unsigned int num_iter;
  // The block number of the current trip indirect 
  blocknum trip_bn;
  // The trip indirect of this iterator if loaded. (NULL if unset or unvisited)
  indirect* trip_ind;
  // The block number of the current double indirect
  blocknum double_bn;
  // The current double indirect of this iterator if loaded. (NULL if unset or
  // unvisited)
  indirect* double_ind;
  // The block number of the current single indirect
  blocknum single_bn;
  // The current single indirect of this iterator if loaded. (NULL if unset or
  // unvisited)
  hash_indirect* single_ind;
  // The dnode of the iterator
  dnode* dir;
}dnode_iter;

/* Call dread with the block index in the given block pointer and the given
	block, returns the value returned by dread */
int dread2(blocknum pointer, char* block);

/* Call dwrite with the block index in the given block pointer and the given
  block, returns the value returned by dwrite */
int dwrite2(blocknum pointer, char* block);

/* Binary search the cachelist for the index of the block with the given
	blocknum in the cache. Returns a cachepos containing a flag for whether or
	not the blocknum was found and a either the index of the found block's
	index in the cache or the index of the cache position right before where
	the block would go */
cachepos cache_find(blocknum block);

/* Compares two blocknums for equality. Returns true if they are structurally
	equal, false otherwise */
bool equals_blocknum(blocknum b0, blocknum b1);

/* Add the given dirent at the given block nummber into the given dnode.
	Return 0 if the ran without a problem. -1 if the DNODE is already full.
	-2 if a file of that name already exists within the DNODE. */
int add_to_dnode(dnode dir, blocknum drntloc, dirent drnt);

/* Hash the given null terminating cstring */
unsigned int hashstr(char* str);

/* Read the block at the given blocknum in the file system. If it is in the cache, grab it
  from the cache. Overwise read from the hard drive and add it to the cache. Returns the block
  or NULL for errors */
char* read_blocknum(blocknum bnum);

/* White the block at the given blocknum in the file system. If it is in the cache, write to
  the value in the cache and after a number of writes, write to the drive. If not within cache,
  create a new value in the cache for the block. Return the value dwrite would return if called. */
int write_blocknum(blocknum bnum, char* block);

/* Given a cstring path, return a NULL-terminated array of cstring tokens of
	each part of the path in order. */
char** parse_path(char* path);

/* Get the dirent that points to the file or directory by the given path.
    Returns NULL if the given file or directory path doesn't actually lead to a
    file or directory. */
dirent* get_from_path(char* path);

#endif
