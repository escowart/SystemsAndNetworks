/*
 * CS3600, Spring 2014
 * Project 2 Starter Code
 * (c) 2013 Alan Mislove
 *
 * This program is intended to format your disk file, and should be executed
 * BEFORE any attempt is made to mount your file system.  It will not, however
 * be called before every mount (you will call it manually when you format 
 * your disk file).
 */

#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "3600fs.h"
#include "disk.h"

void myformat(int size) {
  // Do not touch or move this function
  dcreate_connect();

  if (size >= 4) {
    /* 3600: FILL IN CODE HERE.  YOU SHOULD INITIALIZE ANY ON-DISK
             STRUCTURES TO THEIR INITIAL VALUE, AS YOU ARE FORMATTING
             A BLANK DISK.  YOUR DISK SHOULD BE size BLOCKS IN SIZE. */
    // Make vcb in block 0
             
    vcb block0 = {MAGICNUM, size, ROOT_BLOCKNUM, {2, true}, {size - 1, true}, "muffin button"};
   
    // Make root dnode
    mode_t allpermissions = S_IFREG ^ S_IFDIR ^ S_IRUSR ^ S_IWUSR ^ S_IXUSR ^ S_IRGRP ^ S_IWGRP ^ S_IXGRP ^ S_IROTH ^ S_IWOTH ^ S_IXOTH;
    struct timespec currenttime;
    
    // Fill the rest of the partition with linked free blocks
    for (int i = 2; i < size - 1; i++) {
      freeblock next;
      next.next.block = i+1;
      next.next.valid = true;
      if (dwrite(i, (char*)&next) < 0) 
        perror("Error while writing to disk");
    }
    freeblock next;
    next.next.block = 0;
    next.next.valid = false;
    if (dwrite(size - 1, (char*)&next) < 0) 
        perror("Error while writing to disk");
    }

    // Free nodes from 4 until the end
    dwrite(0, (char*)&block0);
    dnode* root = new_dnode(ROOT_BLOCKNUM, ROOT_BLOCKNUM, NULL, allpermissions);
    
    // Do not touch or move this function
    dunconnect();
    if (size < 4) {
      printf("Error: Number of blocks was not sufficient to format. 4 blocks were required, given %i", size);
    }
}

int main(int argc, char** argv) {
  // Do not touch this function
  if (argc != 2) {
    printf("Invalid number of arguments \n");
    printf("usage: %s diskSizeInBlockSize\n", argv[0]);
    return 1;
  }

  unsigned long size = atoi(argv[1]);
  printf("Formatting the disk with size %lu \n", size);
  myformat(size);
}
