/*
 * CS3600, Spring 2014
 * Project 2 Starter Code
 * (c) 2013 Alan Mislove
 *
 */

#ifndef __3600DNS_H__
#define __3600DNS_H__

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

// Boolean values by name
typedef unsigned int bool;
enum { false, true };


/* An octet */
typedef unsigned char octet;

/*
  DNS packet structure
	+------------+
	|   Header   |	Describes packet type and contained fields
	+------------+
	|  Question  |	Question for the name server
	+------------+
	|   Answer   |	Answers to the question
	+------------+
	| Authority  |	Not used in this project
	+------------+
	| Additional |	Not used in this project
	+------------+
	*/

/*
  DNS Header structure
	  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	|                      ID                       |
	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	|QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	|                    QDCOUNT                    |
	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	|                    ANCOUNT                    |
	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	|                    NSCOUNT                    |
	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	|                    ARCOUNT                    |
	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	where:
	ID is a 16-bit identifier that represents the querying program.
		This should always be 1337 in our case.
	QR is a 1-bit field that represents whether this message is a query (0) or a response (1).
		We should always send 0's and expect 1's.
	OPCODE is a 4-bit field that specifies the type of query.
		We should always use 0, which means standard query.
	AA is a 1-bit flag that represents whether this response is authoritative or not.
		We should always use 0, as we are sending a query.
	TC is a 1-bit flag that represents whether this response is truncated or not.
		We should always use 0, as we are sending a query.
		If we receive a 1, report an error.
	RD is a 1-bit flag that represents the desire to have the server recursively pursue the query.
		We should always use 1, as we want our query to be recursively pursued.
	RA is a 1-bit flag that represents whether the server supports recursive queries or not.
		We should always use 0, as we are sending a query.
		If we receive a 0, report an error.
	Z is a 3-bit field that is reserved for future use.
		We should always use 0, as we are not using this field.
	RCODE is a 4-bit field that is set by the server to notify the sender of errors.
		0 means that there was no error.
		1 means that the name server was unable to interpret the query.
		2 means that the name server was unable to process the query due to a problem on the name server.
		3 means that the name server could not find the requested name.
		4 means that the requested query type is not recognized.
		5 means that the name server refused to perform the specified operation for policy reasons.
		On anything but a 0 or a 3, report an error.
	QDCOUNT is an unsigned 16-bit integer that specifies the number of entires in the question section.
		We should always use 1, as we always have a single question.
	ANCOUNT is an unsigned 16-bit integer that specifies the number of entries in the answer section.
		We should always use 0, as we are providing no answers.
	NSCOUNT is an unsigned 16-bit integer that specifies the number of entries in the authority records section.
		We should always use 0, as we are providing 0 records.
		We should ignore any entries in this section.
	ARCOUNT is an unsigned 16-bit integer that specifies the number of entries in the additional records section.
		We should always use 0, as we are providing 0 additional records.
		We should ignore any entries in this section.
	*/
typedef struct {
	// ID of the querying program
	unsigned int id:16;
	// Do we want the server to recursively pursue this query
	bool rd:1;
	// Is this response truncated
	bool tc:1;
	// Is this response authoratative
	bool aa:1;
	// The type of query
	unsigned int opcode:4;
	// Is this message a query or response
	bool qr:1;	
	// Response errors
	unsigned int rcode:4;
	// Reserved for future use
	unsigned int z:3;
	// Does the server support recursive query pursuing
	bool ra:1;
	// Number of questions that follow
	unsigned int qdcount:16;
	// Number of responses that follow
	unsigned int ancount:16;
	// Number of authoratative records that follow
	unsigned int nscount:16;
	// Number of additional entries that follow
	unsigned int arcount:16;
} dns_header;


/*
  DNS Question/Query Structure
	  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	|                     QNAME                     |
	/                                               /
	/                                               /
	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	|                     QTYPE                     |
	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	|                     QCLASS                    |
	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	where:
	QNAME is an octet specifying the number of octets following this octet, followed by those octets, then terminated with an octet containing the null label of the root that specifies the domain name that was queried.
	QTYPE is a 2-octet code that specifies the type of the query.
		We should always use 0x0001 for A records, 0x000f for MX records, and 0x0002 for NS records.
	QCLASS is a 2-octet code that specifies the query class.
		We should always use 0x0001, as we are always querying internet addresses.
	*/
typedef struct {
  /* The size of the qname */
	unsigned int size_qname;
  /* The octets of the qname */
	octet* qname;
  /* The qtype black data */
	octet qtype_blank;
  /* The qtype */
	octet qtype;
  /* The qclass blank data */
	octet qclass_blank;
  /* The qclass */
	octet qclass;
} dns_query;

/*
  DNS Response/Answers
    0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
  |                     NAME                      |
  /                                               /
  /                                               /
  |                                               |
  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
  |                     TYPE                      |
  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
  |                     CLASS                     |
  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
  |                      TTL                      |
  |                                               |
  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
  |                  RDLENGTH                     |
  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
  /                     RDATA                     /
  /                                               /
  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
  where:
  NAME is an octet specifying the number of octets following this octet, followed by those octets, then terminated with an octet containing the null label of the root that specifies the domain name that was queried.
  TYPE is a 2-octet field that specifies the meaning of the data in the RDATA field.
    0x0001 means that an A record is contained in the RDATA field.
    0x0005 means that a CNAME is contained in the RDATA field.
    0x0002 means that a NS record is contained in the RDATA field.
    0x000f means that a MX record is contained in the RDATA field.
  CLASS is a 2-octet field that specifies the class of data in the RDATA field.
    We should expect this to be 0x0001, as we are expecting to receive responses about internet addresses.
  TTL is the number of seconds the results can be cached.
  RDLENGTH is the length of the RDATA field.
  RDATA is the data of the response, the format of which depends on the TYPE.
    If TYPE is 0x0001 (A record), then RDATA is 4-octets that represent the IP address.
    If TYPE is 0x0005 (CNAME), then RDATA is the name of the alias.
    If TYPE is 0x0002 (NS record), then RDATA is the name of the server.
    If TYPE is 0x000f (MX record), then RDATA is structured:
      +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
      |                  PREFERENCE                   |
      +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
      /                   EXCHANGE                    /
      /                                               /
      +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
      where:
      PREFERENCE is a 16-bit integer that specifies the preference of this mail server.
      EXCHANGE an octet specifying the number of octets following this octet, followed 
      by those octets, then terminated with an octet containing the null label of the 
      root that specifies the domain name that was queried.
  */


/* A structure representing a dns response */
typedef struct {
  /* The type */
  unsigned int rtype:16;
  /* The class */
  unsigned int rclass:16;
  /* The TTL */
  unsigned int ttl;
  /* */
  unsigned int rdlength:16;
  /* The string length */
  unsigned int rd_strlen;
  /* The RDATA each a octet* or a mx_rdata */
  char* rdata_str;
  /* The IP for A record */
  unsigned int ip[4];
  /* The preference for MX record */
  unsigned int pref:16;
}dns_response;

/* DNS Label Pointer
  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
  | 1  1|                OFFSET                   |
  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
  where:
  The first two bits are 1's to specify that this line is a label.
  OFFSET is a 14-bit field that specifies on which line the label is repeated.
  */

/* Constant class and type octets */
const octet BLANK = 0x0000;
const octet A = 0x0001;
const octet MX = 0x000f;
const octet CNAME = 0x0005;
const octet NS = 0x0002;
const octet INTERNET = 0x0001;
const octet MAX_SIZE_OCT = 0xff;

/**
 * This function will print a hex dump of the provided packet to the screen
 * to help facilitate debugging.  In your milestone and final submission, you 
 * MUST call dump_packet() with your packet right before calling sendto().  
 * You're welcome to use it at other times to help debug, but please comment those
 * out in your submissions.
 *
 * DO NOT MODIFY THIS FUNCTION
 *
 * data - The pointer to your packet buffer
 * size - The length of your packet
 */
static void dump_packet(unsigned char *data, int size);


/* Print the error message stored into the variable error_msg is not NULL. */
void print_error();

/* Check for whether an error occurred and if on has and return true.
  Otherwise return false. */
bool check_error();

/* Set the error message */
void set_error(char* errmsg);

/* Set the error message to the given error message if 
  an error has yet to occur*/
void set_if_unset(char* errmsg);

/* Fill the given dns_header with default query values. 
  Returns a negative upon error. */
int fill_header(dns_header* to_fill);

/* Convert the given unsigned int of length 16 and reverse to the two octets within it.
  Returns 0 upon error but can also just return 0. Error should be impossible. */
unsigned int rev_octs(unsigned int num);

/* Is this character allowed in DNS names? */
bool commonchar(const char c);

/* Is the given string a valid dns name? */
bool check_string_validity(const char* string);

/* Get the number of octets that this string would convert to (including length
  octets and the extra blank octet for when their is a odd number of octets).
  Returns 0 upon error and a value of 1 or greater otherwise. */
unsigned int num_octets(const char* string);

/* Convert the given lcstring into the corresponding array of octets such that
  the cstring is tokenized at each '.' in the string. Each of those tokens is
  then converted into an octet token which is lead by a character that holds
  the size of token and procced by all the chars in that token as octets. Is
  the number of octets is odd then an octet with the value of 0 ends the array
  of octets. If successful this function will set the value of derefensed 
  octets variable to a newly allocated octet array on the heap. The function
  will place the size of the octet array into the len variable. The function 
  will return 0 unless an error occured. Upon an error a negative is returned.
  */
int string_to_octets(const char* string, octet** octets, unsigned int* len);

/* Complete the given dns_query with the given string */
int fill_query(dns_query* query, const char* string, const octet type);

/* Parse the input given to the program and store all valid pieces into the
  value of each pointer to mx, ns, server, port, and target into their 
  corresponding variables. If the given input is invalid, return a negative
  number, else return 0. */
int parse_input(int argc, char** argv, bool* mx, bool* ns, char** server,  short* port, char** target);

/* Make a query packet from the given header and given query, and store the
  length of the packet in bytes in the given integer */
unsigned char* make_packet(dns_header* header, dns_query* query, int* size);

/* Read the header of the given packet of the size into the given dns_header.
  Returns the potion immediately after the header or a Negative if an error occured */
int read_header(unsigned char* packet, int packet_size, dns_header* header);

/* Concatinate the first octet to the beginning of the second octet and place the
  result in the destination. */
unsigned int octets_to_uint(octet first8, octet second8);

/* Concatinate the four octets into a unsigned int  */
unsigned int octets_to_uint2(octet first8, octet second8, octet third8, octet fourth8);

/* Check whether the given Header for a response is valid. If not return false
  and set the error message */
bool check_resp_header(dns_header* header);

/* Get the size of the name at the given offset within packet of the size, packet_size.
  And set the offset to after the last actual position in the name. i.e. after 
  the 0 octet or the pointer
  Returns a negative upon error. */
int get_name_size(unsigned char* packet, int packet_size, int* offset);

/* Get the name at the given offset within packet of the size, packet_size.
  Return the name in octet format and put the size into the given octssize pointer.
  Return NULL upon an error. */
char* read_name(unsigned char* packet, int packet_size, int* offset, unsigned int* strsize);

/* Read the query of the given of packet of the given size into the given response
  starting at the given offset. Returns NULL if error otherwise return the qname of the
  query. Put the new value of the offset into the given variable. */
char* read_query(unsigned char* packet, int packet_size, int* offset, dns_query* query);

/* Read the response of the given of packet of the given size into the given response
  starting at the given offset. Returns a negative if error occurred and the new offset otherwise */
int read_response(unsigned char* packet, int packet_size, int offset, dns_response* resp);

/* Print out the given dns_response in accordance with its type and athority.*/
void printf_response(dns_response* response, bool auth);
#endif

