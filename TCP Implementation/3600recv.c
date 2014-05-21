/*
 * CS3600, Spring 2013
 * Project 4 Starter Code
 * (c) 2013 Alan Mislove
 *
 */

#include <math.h>
#include <ctype.h>
#include <time.h>
#include <stdio.h>
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

#include "3600sendrecv.h"

// The currently accepted sequence number
unsigned int sequence = 0;
// The number of ranges that are excepted
unsigned int num_in_sdata = 0;
// The number of positions in the cache
const unsigned int SDATA_SIZE = 10000;
// Cached sequence number and the data
seqdata sdata[10000];

/* Write the given sequence number */
void write_out(unsigned int seq_num, char* data, unsigned int len) {
  // The sequence is directly after the sequence
  if (is_seq_before(seq_num, sequence)) {
    return;
  } else if (sequence == seq_num) {
    mylog("[recv data] %d (%d) %s\n", seq_num, len, "ACCEPTED (in-order)");
    // Sequence in order
    sequence += len;
    // Write the data
    write(1, data, len);
    unsigned int start = 0;
    // Continue iterating through
    
    for (unsigned int i = 0; i < num_in_sdata; i++) {
      if (sequence == sdata[i].seq_num) {
        // The next sequence should be writen if able
        sequence += sdata[i].data_size;
        write(1, sdata[i].data, sdata[i].data_size);
      } else {
        if (start == 0) {
          return;
        }
        sdata[start++] = sdata[i];
      }
    }
    num_in_sdata = start;
  } else {
    mylog("[recv data] %d (%d)\n", seq_num, len);
    // The modification location
    unsigned int loc = 0;
    unsigned int pseq = sequence;
    unsigned int nseq = 0;

    // Go through the cache
    for (; loc < num_in_sdata; loc++) {
      nseq = sdata[loc].seq_num;
      if (pseq < seq_num && seq_num < nseq) {
        // Shift the the list over right to make room
        for (unsigned int j = loc+1; j < SDATA_SIZE && j <= num_in_sdata; j++) {
          sdata[j] = sdata[j-1];
        }
        break;
      }
      pseq = nseq;
    }

    // Set the value at the location 
    if (loc < SDATA_SIZE) {
      sdata[loc].seq_num = seq_num;
      for (unsigned int i = 0; i < len; i++) {
        sdata[loc].data[i] = data[i];
      }
      sdata[loc].data_size = len;
    }

    // Increment num_in_sdata
    if (num_in_sdata < SDATA_SIZE) {
      num_in_sdata++;
    }
  }
}

int main() {
  /**
   * I've included some basic code for opening a UDP socket in C, 
   * binding to a empheral port, printing out the port number.
   * 
   * I've also included a very simple transport protocol that simply
   * acknowledges every received packet.  It has a header, but does
   * not do any error handling (i.e., it does not have sequence 
   * numbers, timeouts, retries, a "window"). You will
   * need to fill in many of the details, but this should be enough to
   * get you started.
   */

  // first, open a UDP socket  
  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  // next, construct the local port
  struct sockaddr_in out;
  out.sin_family = AF_INET;
  out.sin_port = htons(0);
  out.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(sock, (struct sockaddr *) &out, sizeof(out))) {
    perror("bind");
    exit(1);
  }

  struct sockaddr_in tmp;
  int len = sizeof(tmp);
  if (getsockname(sock, (struct sockaddr *) &tmp, (socklen_t *) &len)) {
    perror("getsockname");
    exit(1);
  }

  mylog("[bound] %d\n", ntohs(tmp.sin_port));

  // wait for incoming packets
  struct sockaddr_in in;
  socklen_t in_len = sizeof(in);

  // construct the socket set
  fd_set socks;

  // construct the timeout
  struct timeval t;
  t.tv_sec = 30;
  t.tv_usec = 0;

  // our receive buffer
  int buf_len = 1500;
  void* buf = malloc(buf_len);

  int eof_hit = 0;
  unsigned int eof_seq = 0;
  // wait to receive, or for a timeout
  while (1) {
    FD_ZERO(&socks);
    FD_SET(sock, &socks);

    if (select(sock + 1, &socks, NULL, NULL, &t)) {
      int received;
      if ((received = recvfrom(sock, buf, buf_len, 0, (struct sockaddr *) &in, (socklen_t *) &in_len)) < 0) {
        perror("recvfrom");
        free(buf);
        exit(1);
      }

      //dump_packet(buf, received);

      header *myheader = get_header(buf);
      char *data = get_data(buf);
      unsigned int myh_seq = myheader->sequence;
  
      if (myheader->magic == MAGIC) {
        if (myheader->eof) {
          mylog("[recv eof]\n");
          eof_hit = 1;
          eof_seq = myh_seq;
        } else {
          write_out(myh_seq, data, myheader->length);
        }

        if (eof_hit && sequence == eof_seq) {
          mylog("[send eof ack]\n");
          header *responseheader = make_header(sequence, 0, 1, 1);
          free(buf);
          if (sendto(sock, responseheader, sizeof(header), 0, (struct sockaddr *) &in, (socklen_t) sizeof(in)) < 0) {
            perror("sendto");
            exit(1);
          }
          mylog("[completed]\n");
          exit(0);
        } else {
          mylog("[send ack] %d\n", sequence);
          header *responseheader = make_header(sequence, 0, 0, 1);
          if (sendto(sock, responseheader, sizeof(header), 0, (struct sockaddr *) &in, (socklen_t) sizeof(in)) < 0) {
            perror("sendto");
            free(buf);
            exit(1);
          }
        }
      } else {
        mylog("[recv corrupted packet]\n");
      }
    } else {
      mylog("[error] timeout occurred\n");
      free(buf);
      exit(1);
    }
  }
  free(buf);

  return 0;
}



