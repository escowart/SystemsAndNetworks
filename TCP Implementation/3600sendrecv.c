/*
 * CS3600, Spring 2013
 * Project 4 Starter Code
 * (c) 2013 Alan Mislove
 *
 */


#include "3600sendrecv.h"


// The maximum window size
const unsigned int MAX_WINDOW_SIZE = 2147483648;

const unsigned int MAX_UINT = 4294967295;

unsigned int MAGIC = 0x0bee;

char ts[16];


/* Does the given sequence 0 come before sequence 1 */
int is_seq_before(unsigned int seq0, unsigned int seq1) {
  if (seq0 == seq1) {
    return 0;
  } else if (seq0 < seq1) {
    return (seq1 - seq0) <= MAX_WINDOW_SIZE;
  } else {
    return (seq0 - seq1) >= MAX_WINDOW_SIZE;
  }
}

/* Set the timespec to the current time */
void set_time(struct timeval* tv) {
  gettimeofday(tv, NULL);
}

/**
 * Returns a properly formatted timestamp
 */
char *timestamp() {
  time_t ltime;
  ltime=time(NULL);
  struct tm *tm;
  tm=localtime(&ltime);
  struct timeval tv1;
  gettimeofday(&tv1, NULL);

  sprintf(ts,"%02d:%02d:%02d %03d.%03d", tm->tm_hour, tm->tm_min, tm->tm_sec, (int) (tv1.tv_usec/1000), (int) (tv1.tv_usec % 1000));
  return ts;
}

/**
 * Logs debugging messages.  Works like printf(...)
 */
void mylog(char *fmt, ...) {
  va_list args;
  va_start(args,fmt);
  fprintf(stderr, "%s: ", timestamp());
  vfprintf(stderr, fmt,args);
  va_end(args);
}

/**
 * This function takes in a bunch of header fields and 
 * returns a brand new header.  The caller is responsible for
 * eventually free-ing the header.
 */
header *make_header(int sequence, int length, int eof, int ack) {
  header *myheader = (header *) malloc(sizeof(header));
  myheader->magic = MAGIC;
  myheader->eof = eof;
  myheader->sequence = htonl(sequence);
  myheader->length = htons(length);
  myheader->ack = ack;

  return myheader;
}

/**
 * This function takes a returned packet and returns a header pointer.  It
 * does not allocate any new memory, so no free is needed.
 */
header *get_header(void *data) {
  header *h = (header *) data;
  h->sequence = ntohl(h->sequence);
  h->length = ntohs(h->length);

  return h;
}

/**
 * This function takes a returned packet and returns a pointer to the data.  It
 * does not allocate any new memory, so no free is needed.
 */
char *get_data(void *data) {
  return (char *) data + sizeof(header);
}

/**
 * This function will print a hex dump of the provided packet to the screen
 * to help facilitate debugging.  
 *
 * DO NOT MODIFY THIS FUNCTION
 *
 * data - The pointer to your packet buffer
 * size - The length of your packet
 */
void dump_packet(unsigned char *data, int size) {
    unsigned char *p = data;
    unsigned char c;
    int n;
    char bytestr[4] = {0};
    char addrstr[10] = {0};
    char hexstr[ 16*3 + 5] = {0};
    char charstr[16*1 + 5] = {0};
    for(n=1;n<=size;n++) {
        if (n%16 == 1) {
            /* store address for this line */
            snprintf(addrstr, sizeof(addrstr), "%.4x",
               ((unsigned int)(intptr_t) p-(unsigned int)(intptr_t) data) );
        }
            
        c = *p;
        if (isalnum(c) == 0) {
            c = '.';
        }

        /* store hex str (for left side) */
        snprintf(bytestr, sizeof(bytestr), "%02X ", *p);
        strncat(hexstr, bytestr, sizeof(hexstr)-strlen(hexstr)-1);

        /* store char str (for right side) */
        snprintf(bytestr, sizeof(bytestr), "%c", c);
        strncat(charstr, bytestr, sizeof(charstr)-strlen(charstr)-1);

        if(n%16 == 0) { 
            /* line completed */
            printf("[%4.4s]   %-50.50s  %s\n", addrstr, hexstr, charstr);
            hexstr[0] = 0;
            charstr[0] = 0;
        } else if(n%8 == 0) {
            /* half line: add whitespaces */
            strncat(hexstr, "  ", sizeof(hexstr)-strlen(hexstr)-1);
            strncat(charstr, " ", sizeof(charstr)-strlen(charstr)-1);
        }
        p++; /* next byte */
    }

    if (strlen(hexstr) > 0) {
        /* print rest of buffer if not empty */
        printf("[%4.4s]   %-50.50s  %s\n", addrstr, hexstr, charstr);
    }
}

/* Shift the elements given array starting from start to num_in_arr to the front of the 
array */
void shift_left_tsd(time_seqdata* tsarray, unsigned int start, unsigned int* num_in_arr) {
  if (start == 0) {
    return;
  }
  // Move the elements in the array
  unsigned int i = 0;
  for (; start < *num_in_arr; i++) {
    tsarray[i] = tsarray[start++];
  }
  *num_in_arr = i;
}

/* Is the given sequence number a valid sequence number */
int is_valid_seq(unsigned int seq_check, unsigned int seq_against) {
  if (seq_check == seq_against) {
    return 1;
  }
  unsigned int seq0 = 0;
  unsigned int seq1 = 0;
  if (seq_check < seq_against) {
    seq0 = seq_against;
    seq1 = seq_check;
  } else {
    seq0 = seq_check;
    seq1 = seq_against;
  }
  return (MAX_UINT - seq0) + seq1 < seq0 - seq1;
}