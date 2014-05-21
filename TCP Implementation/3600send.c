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



static int DATA_SIZE = 1460;
// The sequence number of the first packet to not have a response
unsigned int fir_seq = 0;
// The sequence number of the next availible sequence
unsigned int next_seq = 0;
// The number of packets in the cache
unsigned int num_in_spack = 0;
// The maximum number of packets in the cache
const unsigned int MAX_IN_SPACK = 10000;
// The ordered list by sequence number
time_seqdata sent_packet[10000];
// The number of packets confirmed
long long unsigned int num_conf = 0;
// The average round trip time
long long unsigned int rt_time = 5000000;
// The size of the current window
unsigned int window_size = 14600;
// The previous window size
unsigned int prev_window = 1;
// The number of previous timeouts
unsigned int num_time_outs = 0;
// Exponentail 0 or Muliplicitive 1 growth or 2 for none
unsigned int exp_or_mult = 0;
// The resend count
unsigned int resend_count = 0;

void usage() {
  printf("Usage: 3600send host:port\n");
  exit(1);
}

long long int get_diff(struct timeval time0, struct timeval time1) {
  long long int result = 0;
  result += time0.tv_usec;
  result -= time1.tv_usec;
  result += time0.tv_sec * 1000000;
  result -= time1.tv_sec * 1000000;
  return result;
}

/* Add the given packet with the given sequence to the cache */
void add_to_cache(unsigned int sequence, unsigned int eof, char* data, unsigned int data_len) {
  if (num_in_spack < MAX_IN_SPACK) {
    // Get the current time
    struct timeval tv;
    set_time(&tv);
    // Set the next array position
    sent_packet[num_in_spack].sent_time = tv;
    sent_packet[num_in_spack].eof = eof;
    sent_packet[num_in_spack].seq_num = sequence;
    sent_packet[num_in_spack].data_size = data_len;
    for (unsigned int i = 0; i < data_len; i++) {
      sent_packet[num_in_spack].data[i] = data[i];
    }
    num_in_spack++;
  }
}

/* Mod the time lapse average and standard deviation. */
void mod_time_lapse(long long unsigned int dif, struct timeval* t) {
  USE(t);
  if (num_conf == 0) {
    rt_time = 0;
  }
  // Change the average time lapse
  long long unsigned int old_num_conf = num_conf;
  num_conf++;
  rt_time = ((rt_time * old_num_conf) + dif) / num_conf;

  t->tv_sec = rt_time/1000000;
  t->tv_usec = rt_time % 1000000;
}

/* Send the given packet of the given length and the given sequence number */
void send_packet(char* packet, unsigned int eof, unsigned int len, unsigned int seq, int sock, struct sockaddr_in out) {
  // Send the given packet through the port
  if (sendto(sock, packet, len, 0, (struct sockaddr *) &out, (socklen_t) sizeof(out)) < 0) {
    perror("sendto");
    exit(1);
  }
  // Add the packet to the cache
  add_to_cache(seq, eof, (char*) packet, len);
}

/* Resend the given packet with the given parameter */
void resend_packet(time_seqdata* tsdata, int sock, struct sockaddr_in out) {
  // Log the send
  mylog("[resend data] %d (%d)\n", tsdata->seq_num, tsdata->data_size - sizeof(header));
  send_packet(tsdata->data, tsdata->eof, tsdata->data_size, tsdata->seq_num, sock, out);
}

/* Check for a timeout in the cache and resend if necessary with the timevalue */
void check_for_timeout(int sock, struct sockaddr_in out) {
  // None if the cache is empty
  if (num_in_spack == 0) {
    return;
  }
  // Mod up to the mod_to
  unsigned int mod_to = 0;
  // The resend list
  unsigned int next_resend = 0;
  const unsigned int RESEND_SIZE = 1000;
  time_seqdata resend[RESEND_SIZE];
  // Go through the first avaible 1000 elements
  for (unsigned int i = 0; i < num_in_spack && next_resend < RESEND_SIZE; i++) {
    time_seqdata packet = sent_packet[i];
    // Get the difference between the current time and first sent
    struct timeval fir_time = packet.sent_time;
    struct timeval cur_time;
    set_time(&cur_time);
    long long int dif = get_diff(cur_time, fir_time);
    // Time last has occurred
    if (dif < (long long int) (3 * rt_time)) {
      break;
    }
    resend[next_resend++] = packet;
    // Increment the mod_to number and the number of timeouts
    num_time_outs++;
    mod_to++;
  }
  // Shift the sent packets left
  shift_left_tsd(sent_packet, mod_to, &num_in_spack);
  // Resend each packets
  for (unsigned int i = 0; i < next_resend; i++) {
    resend_packet(&(resend[i]), sock, out);
  }
}


/* Confirm all the packets up to the given sequence */
void confirm_to(unsigned int sequence, struct timeval* t) {
  unsigned int next_pos = 0;
  unsigned int i = 0;
  // Iterate over the packets to find and packets
  for (; i < num_in_spack; i++) {
    time_seqdata tsdata = sent_packet[i];
    if (!is_seq_before(tsdata.seq_num, sequence)) {
      sent_packet[next_pos++] = sent_packet[i];
    } else {
      struct timeval cur_time;
      set_time(&cur_time);
      long long int dif = get_diff(cur_time, sent_packet[i].sent_time);
      mod_time_lapse((long long unsigned int) dif, t);
    }
  }
  num_in_spack = next_pos;
  num_time_outs = 0;
}

/* Change the window size */
void change_window() {
  if (num_time_outs > 4) {
    exp_or_mult++;
    window_size = prev_window;
    if (exp_or_mult > 1) {
      window_size--;
    }
  } else if (exp_or_mult == 0) {
    prev_window = window_size;
    window_size *= 1.1;
  } else if (exp_or_mult) {
    prev_window = window_size;
    window_size += 2;
  }
  // Insure the windoew is not greater than the limits
  if (window_size > DATA_SIZE * MAX_IN_SPACK) {
    window_size = DATA_SIZE * MAX_IN_SPACK;
  } else if (window_size < (unsigned int) DATA_SIZE) {
    window_size = DATA_SIZE;
  }
}

/**
 * Reads the next block of data from stdin
 */
int get_next_data(char *data, int size) {
  return read(0, data, size);
}

/**
 * Builds and returns the next packet, or NULL
 * if no more data is available.
 */
void *get_next_packet(int sequence, int *len) {
  // Allocate the data
  char *data = malloc(DATA_SIZE);
  int data_len = get_next_data(data, DATA_SIZE);

  if (data_len == 0) {
    free(data);
    return NULL;
  }

  // Add the header to the data
  header *myheader = make_header(sequence, data_len, 0, 0);
  void *packet = malloc(sizeof(header) + data_len);
  memcpy(packet, myheader, sizeof(header));
  memcpy(((char *) packet) +sizeof(header), data, data_len);

  free(data);
  free(myheader);

  *len = sizeof(header) + data_len;

  return packet;
}

/* Send the next avaible packet from stdin to the given sock and sock_addr */
int send_next_packet(int sock, struct sockaddr_in out) {
  // Construct a new packet
  int packet_len = 0;
  void *packet = get_next_packet(next_seq, &packet_len);

  if (packet == NULL) 
    return 0;
  // Log the send
  mylog("[send data] %d (%d)\n", next_seq, packet_len - sizeof(header));
  // Sent the send
  send_packet((char*) packet, 0, packet_len, next_seq, sock, out);
  next_seq += packet_len - sizeof(header);
  // True to continue
  return 1;
}

void send_final_packet(int sock, struct sockaddr_in out) {
  header *myheader = make_header(next_seq, 0, 1, 0);
  mylog("[send eof]\n");

  // Sent the send
  send_packet((char*) myheader, 1, sizeof(header), next_seq, sock, out);
}

/* Get the size of the data sent already */
unsigned int get_size_sent() {
  if (fir_seq == next_seq) {
    return 0;
  } else if (fir_seq < next_seq) {
    return next_seq - fir_seq;
  } else {
    long int result = MAX_UINT + fir_seq - next_seq;
    return (unsigned int) result;
  }
}

/* Resend if this is the third time the fir_seq has been acknoledged */
void resend(int sock, struct sockaddr_in out) {
  int hit = 0;
  time_seqdata packet;
  // Find the packet and remove it then resend
  for (unsigned int i = 0; i < num_in_spack; i++) {
    if (hit && i != 0) {
      sent_packet[i-1] = sent_packet[i];
    } else {
      if (sent_packet[i].seq_num == fir_seq) {
        packet = sent_packet[i];
        hit = 1;
      }
    }
  }
  // Resend if found
  if (hit) {
    num_in_spack--;
    resend_packet(&packet, sock, out);
  }
}

int main(int argc, char *argv[]) {
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

  // extract the host IP and port
  if ((argc != 2) || (strstr(argv[1], ":") == NULL)) {
    usage();
  }

  char *tmp = (char *) malloc(strlen(argv[1])+1);
  strcpy(tmp, argv[1]);

  char *ip_s = strtok(tmp, ":");
  char *port_s = strtok(NULL, ":");
 
  // first, open a UDP socket  
  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  // next, construct the local port
  struct sockaddr_in out;
  out.sin_family = AF_INET;
  out.sin_port = htons(atoi(port_s));
  out.sin_addr.s_addr = inet_addr(ip_s);

  // socket for received packets
  struct sockaddr_in in;
  socklen_t in_len = sizeof(in);

  // construct the socket set
  fd_set socks;

  // construct the timeout
  struct timeval t;
  t.tv_sec = 30;
  t.tv_usec = 0;

  int eof_hit = 0;
  int cont_send = 1;
  //mylog("Cont Send: %i\tEOF Hit%i\n", cont_send, eof_hit);
  while (cont_send || num_in_spack != 0) {
    // Continue Ssend packets
    while (cont_send && get_size_sent() < window_size && num_in_spack < MAX_IN_SPACK) {
      // Check For eof hit
      if (eof_hit) {
        send_final_packet(sock, out);
        cont_send = 0;
      } else {
        // Send the next packet
        if (!send_next_packet(sock, out)) {
          eof_hit = 1;
        }
      }
    }

    FD_ZERO(&socks);
    FD_SET(sock, &socks);
    int mod_window = 1;

    // wait and recieve
    if (select(sock + 1, &socks, NULL, NULL, &t)) {
      unsigned char buf[10000];
      int buf_len = sizeof(buf);
      int received;
      if ((received = recvfrom(sock, &buf, buf_len, 0, (struct sockaddr *) &in, (socklen_t *) &in_len)) < 0) {
        perror("recvfrom");
        exit(1);
      }

      header *myheader = get_header(buf);
      if (myheader->magic == MAGIC) {
        if (myheader->eof) {
          mylog("[recv eof ack]\n");
          break;
        }
        mylog("[recv ack %d]\n", myheader->sequence);
        if (fir_seq == myheader->sequence) {
          resend(sock, out);
          mod_window = 0;
        } else if (is_seq_before(fir_seq, myheader->sequence)
          && (is_seq_before(myheader->sequence, next_seq) || (myheader->sequence == next_seq))) {
          confirm_to(myheader->sequence, &t);
          fir_seq = myheader->sequence;
        } else {
          mylog("[recv corrupted ack] %x %d\n", MAGIC, fir_seq);
        }
      } else {
        mylog("[recv corrupted ack] %x %d\n", MAGIC, fir_seq);
      }
    }
    check_for_timeout(sock, out);
    if (mod_window) {
      change_window();
    }
  }
  

  mylog("[completed]\n");

  return 0;
}

