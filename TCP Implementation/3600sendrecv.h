/*
 * CS3600, Spring 2013
 * Project 4 Starter Code
 * (c) 2013 Alan Mislove
 *
 */

#ifndef __3600SENDRECV_H__
#define __3600SENDRECV_H__

#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define USE(x) (x) = (x)

extern const unsigned int MAX_UINT;
extern const unsigned int MAX_WINDOW_SIZE;

typedef struct header_t {
  unsigned int magic:14;
  unsigned int ack:1;
  unsigned int eof:1;
  unsigned short length;
  unsigned int sequence;
} header;

/* A Structure Representing segmentation number and corresponding data */
typedef struct s_seqdata {
  // The segmentation number
  unsigned int seq_num;
  // The size of the data field
  unsigned int data_size;
  // The maximum value
  char data[1460];
}seqdata;

/* A Structure Representing a timestamp, sequence number and the corresponding data */
typedef struct s_time_seqdata {
  // The time the packet was sent
  struct timeval sent_time;
  // Whether this packet is the eof
  unsigned int eof;
  // The segmentation number
  unsigned int seq_num;
  // The size of the data field
  unsigned int data_size;
  // The maximum value
  char data[1500];
}time_seqdata;

unsigned int MAGIC;

/* Does the given sequence 0 come before sequence 1 */
int is_seq_before(unsigned int seq0, unsigned int seq1);

void dump_packet(unsigned char *data, int size);
header *make_header(int sequence, int length, int eof, int ack);
header *get_header(void *data);
char *get_data(void *data);
char *timestamp();
void mylog(char *fmt, ...);
void shift_left_tsd(time_seqdata* tsarray, unsigned int start, unsigned int* num_in_arr);

/* Is the given sequence number a valid sequence number */
int is_valid_seq(unsigned int seq_check, unsigned int seq_against);

/* Set the timespec to the current time */
void set_time(struct timeval* tv);

#endif

