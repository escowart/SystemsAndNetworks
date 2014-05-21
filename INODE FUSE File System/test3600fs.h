#include "3600fs.h"

#ifndef TEST3600FS_H
#define TEST3600FS_H

#define USE(x) (x) = (x)

/* Is the vcb we have valid? */
extern bool vcb_valid;

/* The volume control block of the partition. */
extern vcb drive;

/* The root directory of the disk. */
extern dnode root;

/* Number of blocks currently stored in the cache. */
extern unsigned int num_stored;

/* Maximum size of the cache. */
extern const unsigned int CACHE_MAX;

/* Head of the cache list. */
extern listlink* head;

/* Tail of the cache list. */
extern listlink* tail;

/* Cache array. */
extern listlink** cache;

/* Test the mounting of the vcd, the root, and the node */
void testmount();


#endif