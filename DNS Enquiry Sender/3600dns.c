/*
 * CS3600, Spring 2014
 * Project 3 Starter Code
 * (c) 2013 Alan Mislove
 *
 */

#include "3600dns.h"

#define USE(x) (x) = (x)

/* A bool whether an error has occurred. */
bool error_occ;

/* The error message if an error has occurred */
char* error_msg;

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
static void dump_packet(unsigned char *data, int size) {
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
               (unsigned int) ((size_t)p-(size_t)data) );
        }
            
        c = *p;
        if (isprint(c) == 0) {
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

/* Print the error message stored into the variable error_msg is not NULL. */
void print_error() {
  if (error_msg != NULL) {
    printf("ERROR\t%s\n", error_msg);
  } else {
    printf("ERROR\tUnknown Error Occurred\n");
  }
}

/* Check for whether an error occurred and if on has and return true.
  Otherwise return false. */
bool check_error() {
  if (error_occ) {
    print_error();
    return true;
  }
  return false;
}

/* Set the error message */
void set_error(char* errmsg) {
  error_msg = errmsg;
  error_occ = true;
}

/* Set the error message to the given error message if 
  an error has yet to occur*/
void set_if_unset(char* errmsg) {
  if (!error_occ) {
    set_error(errmsg);
  }
}

/* Fill the given dns_header with default query values. 
  Returns a negative upon error. */
int fill_header(dns_header* to_fill) {
  if (to_fill == NULL) {
    set_error("fill_header: Given NULL.");
    return -1;
  }
  to_fill->id = rev_octs(1337);
  to_fill->qr = 0;
  to_fill->opcode = 0;
  to_fill->aa = false;
  to_fill->tc = false;
  to_fill->rd = true;
  to_fill->ra = false;
  to_fill->z = 0;
  to_fill->rcode = 0;
  to_fill->qdcount = rev_octs(1);
  to_fill->ancount = rev_octs(0);
  to_fill->nscount = rev_octs(0);
  to_fill->arcount = rev_octs(0);
  return 0;
}

/* Convert the given unsigned int of length 16 and reverse to the two octets within it.
  Returns 0 upon error but can also just return 0. Error should be impossible. */
unsigned int rev_octs(unsigned int num) {
  // Remove the first 16 bits from the num
  num &= 0xffff;
  // Put the last 8 bits into first8
  unsigned int first8 = (octet)(num & 0xff);
  num &= 0xff00;
  // Put the second to last 8 bits into second8 
  octet second8 = (octet)(num >> 8);
  return octets_to_uint(first8, second8);
}

/* Is this character allowed in DNS names? */
bool commonchar(const char c) {
  return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || (c == '-') || (c == '_');
}

/* Is the given string a valid dns name? */
bool check_string_validity(const char* string) {
  // Check for NULL
  if (string == NULL) {
    set_error("check_string_validity: NULL passed in.");
    return false;
  }
  int len = strlen(string);
  // Check for empty string and insure that there is not a '.' at the beginning 
  //  of the string and '-' at the beginning and end
  if (string[0] == '\0' || string[0] == '.' || string[0] == '-' || string[len] == '-') {
    return false;
  }
  bool prevdot = false;
  // Iterate through each element of string and insure that their are not
  //  two '.' back to back and that all characters are allowed
  for (int i = 0; i < len; i++) {
    if (string[i] == '.') {
      if (prevdot) {
        return false;
      } else {
        prevdot = true;
      }
    } else if (commonchar(string[i])) {
      prevdot = false;
    } else {
      return false;
    }
  }
  // Check whether the string ends with a '.'
  if (prevdot) {
    return false;
  }
  return true;
}

/* Get the number of octets that this string would convert to (including length
  octets and the extra blank octet for when their is a odd number of octets).
  Returns 0 upon error and a value of 1 or greater otherwise. */
unsigned int num_octets(const char* string) {
  if (string == NULL) {
    set_error("num_in_octets: Given NULL.");
    return 0;
  }
  return strlen(string) + 2;
}

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
int string_to_octets(const char* string, octet** octets, unsigned int* len) {
  // Check for NULL
  if (string == NULL || len == NULL) {
    set_error("string_to_octets: Given at least one NULL.");
    return -1;
  }
  // Get the number of octets
  *len = num_octets(string);
  if (*len == 0) {
    // Should never happen. But check anyway.
    set_error("string_to_octets: NULL passed into num_octets.");
    return -2;
  }
  // Allocate the heap variable for the octets
  octet* octs = (octet*) calloc(*len, sizeof(octet));
  if (octets == NULL) {
    set_error("string_to_octets: calloc failed.");
    return -3;
  }
  // The size of the token currently being iterated through
  octet size = 0;
  // The current position in the octets array: the 0th position is the first size
  int oct_pos = 1;
  // The position in the octets of the size octet of the current token
  int size_pos = 0;
  // 
  unsigned int i = 0;
  // Tokenize the string by splitting the string at each '.'
  do {
    char next = string[i++];
    // Upon finding the next . Get the previous size 
    if (next == '.' || next == '\0') {
      // Check whether the size of the current token is not 0
      if (size == 0) {
        set_error("Token Size Not Permitted to be 0.");
        free(octs);
        return -4;
      }
      // Set the size position of the current token
      octs[size_pos] = size;
      if (next == '\0') {
        break;
      }
      // Reset the size of the current token to 0 and the position of the size octet to the current location in the octet array
      size = 0;
      size_pos = oct_pos++;
    } else {
      // Check whether the size of the current token is the maximum
      if (MAX_SIZE_OCT == size) {
        sprintf(error_msg, "Token beyond the maximum length of %u.\n", (unsigned int) MAX_SIZE_OCT);
        error_occ = true;
        free(octs);
        return -5;
      }
      // Set the current position in the octet to the current char in the string
      octs[oct_pos++] = (octet) next;
      size++;
    }
  } while(true);
  // Sets the last octet to 0
  octs[oct_pos] = BLANK;
  *octets = octs;
  return 0;
}

/* Complete the given dns_query with the given string */
int fill_query(dns_query* query, const char* string, const octet type) {
  int sto_result = string_to_octets(string, &(query->qname), &(query->size_qname));
  if (sto_result < 0) {
    set_if_unset("fill_query: string_to_octets failed.");
    return -2;
  } 
  query->qtype_blank = BLANK;
  query->qtype = type;
  query->qclass_blank = BLANK;
  query->qclass = INTERNET;
  return 0;
}

/* Parse the input given to the program and store all valid pieces into the
  value of each pointer to mx, ns, server, port, and target into their 
  corresponding variables. If the given input is invalid, return a negative
  number, else return 0. */
int parse_input(int argc, char** argv, bool* mx, bool* ns, char** server,  short* port, char** target) {
  USE(mx);
  USE(ns);
  USE(target);
  USE(server);
  char* server_port = NULL;
  // Check for a NULL in argv
  for (int i = 0; i < argc; i++) {
    if (argv[i] == NULL) {
      set_error("NULL Argument Found in Command Line Arguments.");
      return -1;
    }
  }
  // Check the number of inputs
  if (argc != 3 && argc != 4) {
    // Error invalid number of inputs in argv
    set_error("Incorrect Number of Comand Line Arguments.");
    return -2;
  } else if (argc == 4) {
    // Check for the flags -ns and -mx
    if (strcmp(argv[1], "-ns") == 0) {
      *ns = true;
    } else if (strcmp(argv[1], "-mx") == 0) {
      *mx = true;
    } else {
      // Error the second argument in argv is not a valid flag
      sprintf(error_msg, "Incorrect Flag Passed:\tGiven: %s.", argv[1]);
      error_occ = true;
      return -3;
    }
    // Set the server variable(tempary vairable) and target for 4 args
    server_port = argv[2];
    *target = argv[3];
  } else {
    // Set the server variable(tempary vairable) and target for 3 args
    server_port = argv[1];
    *target = argv[2];
  }
  // Make sure the first char is '@'
  if (server_port[0] != '@') {
    set_error("Server Name Not Preceeded by @ Character.");
    return -4;
  }
  // Find the colin within the serve-port concatination.
  int colon_pos = 0;
  int len = strlen(server_port);
  for (; colon_pos < len; colon_pos++) {
    if (server_port[colon_pos] == ':') {
      // Set the ':' as the end of cstring
      server_port[colon_pos] = '\0';
      break;
    }
  }
  // Check for the correct position of the ':'
  if (colon_pos + 1 == len || colon_pos == 0) {
    set_error("Colin Found at Beginning or End of Server Name.");
    return -5;
  } else if (colon_pos == len) {
    // If there is no colin set the port to 53
    *port = 53;
  } else {
    // Check for invalid port and set the port 
    if (strlen(server_port+colon_pos+1) > 5) {
      sprintf(error_msg, "Port number is Greater than 5 Characters\tGiven: %s.", server_port+colon_pos+1);
      error_occ = true;
      return -6;
    }
    *port = (short) strtol(server_port+colon_pos+1, NULL, 10);
  }
  // Move the server pointer up one character
  *server = server_port+1;
  return 0;
}

/* Make a query packet from the given header and given query, and store the
  length of the packet in bytes in the given integer */
unsigned char* make_packet(dns_header* header, dns_query* query, int* size) {
  // Check for NULL inputs
  if (header == NULL || query == NULL || size == NULL || query->qname == NULL) {
    set_error("make_packet: NULL passed in.");
    return NULL;
  }
  // Convert the header to characters and intialize the size
  unsigned char* header_data = (unsigned char*) header;
  *size = (unsigned int) sizeof(dns_header) + 4 + query->size_qname;
  // Allocate the packet
  unsigned char* result = (unsigned char*) calloc(*size, sizeof(octet));
  if (result == NULL) {
    set_error("make_packet: calloc failed.");
    return NULL;
  }
  // Add each char in the converted header to the packet
  unsigned int i = 0;
  for (; i < (unsigned int) sizeof(dns_header); i++) {
    result[i] = header_data[i];
  }
  // The add each octet in the query name to the packet
  for (unsigned int j = 0; j < query->size_qname; j++) {
    result[i++] = query->qname[j];
  }
  free(query->qname);
  // Add the other fields from query
  result[i++] = query->qtype_blank;
  result[i++] = query->qtype;
  result[i++] = query->qclass_blank;
  result[i++] = query->qclass;
  return result;
}

/* Read the header of the given packet of the size into the given dns_header.
  Returns the potion immediately after the header or a Negative if an error occured */
int read_header(unsigned char* packet, int packet_size, dns_header* header) {
  if (packet == NULL || header == NULL) {
    set_error("read_header: NULL passed in.");
    return -1;
  }
  if (packet_size < 96) {
    set_error("The Packet is Smaller than Required Header Size.");
    return -2;
  }
  header->id = octets_to_uint(packet[0], packet[1]);
  // The 3rd Octet parsed
  octet oct2 = (octet) packet[2];
  header->qr = (bool) (oct2 >> 7);
  oct2 &= 0x7f;
  header->opcode = (unsigned int) (oct2 >> 3);
  oct2 &= 0x7;
  header->aa = (bool) (oct2 >> 2);
  oct2 &= 0x3;
  header->tc = (bool) (oct2 >> 1);
  oct2 &= 0x1;
  header->rd = (bool) oct2;
  // The 4th Octet parsed
  octet oct3 = (octet) packet[3];
  header->ra = (bool) (oct3 >> 7);
  oct3 &= 0x7f;
  header->z = (unsigned int) (oct3 >> 4);
  oct3 &= 0xf;
  header->rcode = (unsigned int) oct3;

  header->qdcount = octets_to_uint(packet[4], packet[5]);
  header->ancount = octets_to_uint(packet[6], packet[7]);
  header->nscount = octets_to_uint(packet[8], packet[9]);
  header->arcount = octets_to_uint(packet[10], packet[11]);
  return 12;
}

/* Concatinate the first octet to the beginning of the second octet and place the
  result in the destination. */
unsigned int octets_to_uint(octet first8, octet second8) {
  return (((unsigned int) first8) << 8) + ((unsigned int) second8);
}

/* Concatinate the four octets into a unsigned int  */
unsigned int octets_to_uint2(octet first8, octet second8, octet third8, octet fourth8) {
  unsigned int result = octets_to_uint(first8, second8);
  result = (result << 8) + (unsigned int) third8;
  result = (result << 8) + (unsigned int) fourth8;
  return result;
}

/* Check whether the given Header for a response is valid. If not return false
  and set the error message */
bool check_resp_header(dns_header* header) {
  if (header->id != 1337) {
    printf("%i\n", header->id);
    set_error("Response Has Invalid ID.");
    return false;
  } else if (header->qr == 0) {
    set_error("Response Was A Query.");
    return false;
  } else if (header->opcode != 0) {
    set_error("Opcode Not Set to 0.");
    return false;
  } else if (header->tc == 1) {
    set_error("The Packet Should Not Be Truncated.");
    return false;
  } else if (header->ra == 0) {
    set_error("Recursion Not Available Grom The Server.");
    return false;
  } else if (header->rcode == 1) {
    set_error("Formatting Error: Query Unable To Be Interpreted By The Name Server.");
    return false;
  } else if (header->rcode == 2) {
    set_error("Server Failure: Problem Occurred Within The Name Server When Processing the Query.");
    return false;
  } else if (header->rcode == 4) {
    set_error("Not Implemented: The Query Is Not Supported By The Name Server.");
    return false;
  } else if (header->rcode == 5) {
    set_error("Refused: The Name Serve Has Refused To Preform The Query For Policy Reasons.");
    return false;
  } else {
    return true;
  }
}

/* Get the size of the name at the given offset within packet of the size, packet_size.
  And set the offset to after the last actual position in the name. i.e. after 
  the 0 octet or the pointer
  Returns a negative upon error. */
int get_name_size(unsigned char* packet, int packet_size, int* offset) {
  int result = 0;
  octet token_size = 0;
  // Has the offset not been set
  bool not_set = true;
  // Copy of the offset
  int off = *offset;
  for (; off < packet_size; off++) {
    if (result >= packet_size) {
      // An Error Occurred with Size of the String
      return -1;
    }
    // Get the next element in the packet
    octet oct = (octet) packet[off];
    if (token_size == 0) {
      // Not currently within a token
      if (oct == 0) {
        // End of name reached
        if (not_set) {
          *offset = off + 1;
        }
        return result;
      } else if (oct >> 6 == 3) {
        // Hit a pointer
        if (++off < packet_size) {
          // Valid Offset End
          if (not_set) {
            *offset = off + 1;
            not_set = false;
          }
          octet next = (octet) packet[*offset];
          off = octets_to_uint((oct & 0x3f), next);
          continue;
        } else {
          // Pointer Cut off at end of packet
          set_error("Pointer Cut Off at End of Packet.");
          return -2;
        }
      } else if (oct < MAX_SIZE_OCT) {
        // Valid Token Size Reached
        token_size = oct;
        result++; 
      } else {
        // Invalid Size Reached
        sprintf(error_msg, "Invalid Token Size of %u.", (unsigned int) oct);
        error_occ = true;
        return -1;
      }
    } else {
      result++; 
      token_size--;
    }
  }
  // End of packet reached
  set_error("End of Packet Reached Without Ending a Name.");
  return -1;
}

/* Get the name at the given offset within packet of the size, packet_size.
  Return the name in octet format and put the size into the given octssize pointer.
  Return NULL upon an error. */
char* read_name(unsigned char* packet, int packet_size, int* offset, unsigned int* strsize) {
  // Get a copy of the value in the offset pointer
  int off = *offset;
  // Get the size of name
  *strsize = get_name_size(packet, packet_size, offset);
  if (*strsize == 0) {
    // Empty String
    set_error("Invalid Name of Empty String.");
    return NULL;
  }
  // Allocate the result
  char* result = (char*) calloc(*strsize, sizeof(char));
  if (result == NULL) {
    set_error("Calloc Has Failed.");
    return NULL;
  }
  // The current position in the string
  int pos_str = 0;
  // The size of the current token
  octet token_size = 0;
  // Iterate through the packet
  for (; off < packet_size; off++) {
    // Get the next element in the packet
    octet oct = (octet) packet[off];
    if (token_size == 0) {
      // Not currently within a token
      if (oct == 0) {
        result[pos_str] = '\0';
        return result;
      } else if (oct >> 6 == 3) {
        // Hit a pointer
        if (++off < packet_size) {
          // Valid Offset End
          octet next = (octet) packet[off];
          off = octets_to_uint((oct & 0x3f), next) - 1;
          continue;
        } else {
          // Pointer Cut off at end of packet
          set_error("Pointer Cut Off at End of Packet.");
          free(result);
          return NULL;
        }
      } else if (oct < MAX_SIZE_OCT) {
        if (pos_str != 0) {
          result[pos_str++] = '.';
        }
        // Valid Token Size Reached
        token_size = oct;
      } else {
        // Invalid Size Reached
        sprintf(error_msg, "Invalid Token Size of %u.", (unsigned int) oct);
        error_occ = true;
        free(result);
        return NULL;
      }
    } else {
      if (!commonchar((char) oct)) {
        set_error("Invalid Character.");
        free(result);
        return NULL;
      }
      result[pos_str++] = (char) oct; 
      token_size--;
    }
  }
  // End of packet reached
  set_error("End of Packet Reached Without Ending a Name.");
  free(result);
  return NULL;
}

/* Read the query of the given of packet of the given size into the given response
  starting at the given offset. Returns NULL if error otherwise return the qname of the
  query. Put the new value of the offset into the given variable. */
char* read_query(unsigned char* packet, int packet_size, int* offset, dns_query* query) {
  unsigned int strsize;
  // Read the name of the query
  char* result = read_name(packet, packet_size, offset, &strsize);
  if (result == NULL) {
    set_error("Invalid Query.");
    return NULL;
  }
  // Set all other fields
  query->qtype_blank = packet[*offset];
  *offset += 1;
  query->qtype = packet[*offset];
  *offset += 1;
  query->qclass_blank = packet[*offset];
  *offset += 1;
  query->qclass = packet[*offset];
  *offset += 1;
  return result;
}

/* Read the response of the given of packet of the given size into the given response
  starting at the given offset. Returns a negative if error occurred and the new offset otherwise */
int read_response(unsigned char* packet, int packet_size, int offset, dns_response* resp) {
  // Move past the name of the response
  int gns = get_name_size(packet, packet_size, &offset);
  if (gns <= 0) {
    // Error in the name
    set_error("Packet's Response Name is Invalid.");
    return -1;
  }
  // Check For 8 availible octets
  if (offset + 8 > packet_size) {
    set_error("Packet's Response Not Long Enough.");
    return -2;
  }
  // Set the type and class
  octet rt0 = (octet)packet[offset];
  offset += 1;
  octet rt1 = (octet)packet[offset];
  offset += 1;
  resp->rtype = octets_to_uint(rt0, rt1);
  octet rc0 = (octet)packet[offset];
  offset += 1;
  octet rc1 = (octet)packet[offset];
  offset += 1;
  resp->rclass = octets_to_uint(rc0, rc1);
  // Check for the class of Internet
  if (resp->rclass != INTERNET) {
    set_error("Packet's Class is not Internet.");
    return -3;
  }
  // Get the ttl
  octet ttl0 = (octet)packet[offset];
  offset += 1;
  octet ttl1 = (octet)packet[offset];
  offset += 1;
  octet ttl2 = (octet)packet[offset];
  offset += 1;
  octet ttl3 = (octet)packet[offset];
  offset += 1;
  resp->ttl = octets_to_uint2(ttl0, ttl1, ttl2, ttl3);
  octet rdlen0 = (octet)packet[offset];
  offset += 1;
  octet rdlen1 = (octet)packet[offset];
  offset += 1;
  resp->rdlength = octets_to_uint(rdlen0, rdlen1);
  // Check for MX type first
  if (resp->rtype == MX) {
    // Check for bits for preference
    if (offset + 2 > packet_size) {
      set_error("Packet's Too Small For a Preferense.");
      return -5;
    }
    // Set the preference
    octet pref0 = (octet)packet[offset];
    offset += 1;
    octet pref1 = (octet)packet[offset];
    offset += 1;
    resp->pref = octets_to_uint(pref0, pref1);
  }
  // Check for the different types of response
  if (resp->rtype == NS || resp->rtype == CNAME || resp->rtype == MX) {
    // Same operations for NS and CNAME types
    // Get the name and the size
    unsigned int strsize;
    resp->rdata_str = read_name(packet, packet_size, &offset, &strsize);
    resp->rd_strlen = strsize;
    // Check for error in read_name
    if (resp->rdata_str == NULL) {
      set_error("Packet's Alias or Server Name is Invalid.");
      return -4;
    }
  } else if (resp->rtype == A) {
    // Is an A-Record and check for 4 availible octets
    if (offset + 4 > packet_size) {
      set_error("Packet's Too Small For a IP.");
      return -7;
    }
    // Set the ip
    for (int i = 0; i < 4; i++) {
      resp->ip[i]= packet[offset++];
    }
  } else {
    // Invalid type
    set_error("Packet's Response Is of Either Unimplemented or Invalid Type.");
    return -8;
  }
  return offset;
}

/* Print out the given dns_response in accordance with its type and athority. */
void printf_response(dns_response* response, bool auth) {
  if (response->rtype == A) {
    printf("IP\t");
    for (int i = 0; i < 3; i++) {
      printf("%u.", response->ip[i]);
    }
    printf("%u", response->ip[3]);
  } else if (response->rtype == CNAME) {
    printf("CNAME\t%s", response->rdata_str);
  } else if (response->rtype == MX) {
    printf("MX\t%s\t%u", response->rdata_str, response->pref);
  } else if (response->rtype == NS) {
    printf("NS\t%s", response->rdata_str);
  } else {
    printf("ERROR\tInvalid Response Type\n");
    return;
  }
  // Print the authoruty
  if (auth) {
    printf("\tauth\n");
  } else {
    printf("\tnonauth\n");
  }
}

/* Run the program */
int main(int argc, char *argv[]) {
  error_occ = false;
  error_msg = NULL;

  // process the arguments
  bool mx = false;
  bool ns = false;
  char* server = NULL;
  short port = 0;
  char* target = NULL;
  // Parse the input and check for errors
  int sub_result = parse_input(argc, argv, &mx, &ns, &server, &port, &target);
  if (check_error()) {
    return -1;
  } else if (sub_result < 0) {
    printf("ERROR\tParsing Input Failed.\n");
    return -1;
  }

  // Check whether the target is the valid type of address
  bool check = check_string_validity(target);
  if (check_error()) {
    return -2;
  } else if (!check) {
    if (!check_error()) {
      printf("ERROR\tTarget, %s, Is of Invalid Format\n", target);
    }
    return -2;
  }

  // construct the DNS request
  dns_header header;
  sub_result = fill_header(&header);
  if (check_error()) {
    return -3;
  } else if (sub_result < 0) {
    printf("ERROR\tSetting Up the Header Has Failed.\n");
    return -3;
  }

  dns_query query;
  // Set the type of request
  octet type = A;
  if (mx) {
    type = MX;
  } else if (ns) {
    type = NS;
  }

  // Fill in the query
  sub_result = fill_query(&query, target, type);
  if (check_error()) {
    return -4;
  } else if (sub_result < 0) {
    printf("ERROR\tSetting Up the Query Has Failed.\n");
    return -4;
  }

  // Create the packet from the header and query
  int packet_size = 0;
  unsigned char* our_packet = (unsigned char*) make_packet(&header, &query, &packet_size);
  if (check_error()) {
    return -5;
  } else if (our_packet == NULL) {
    printf("ERROR\tPacket Failed to be Constructed\n");
    return -5;
  }

  // first, open a UDP socket  
  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock < 0) {
    free(query.qname);
    printf("ERROR\tSetting Up the Socket Has Failed.\n");
    return -6;
  }

  // next, construct the destination address
  struct sockaddr_in out;
  out.sin_family = AF_INET;
  out.sin_port = htons(port);
  out.sin_addr.s_addr = inet_addr(server);


  /* Dump the packet, our_packet, of the size, packet_size.*/
  dump_packet(our_packet, packet_size);

  /* Send the packet, our_packet, of the size, packet_size to the address in out. */
  if (sendto(sock, our_packet, packet_size, 0, (struct sockaddr*)&out, sizeof(out)) < 0) {
    printf("ERROR\tPacket Failed to be Sent.\n");
    return -7;
  }

  // wait for the DNS reply (timeout: 5 seconds)
  struct sockaddr_in in;
  socklen_t in_len;

  // construct the socket set
  fd_set socks;
  FD_ZERO(&socks);
  FD_SET(sock, &socks);

  // construct the timeout
  struct timeval t;
  t.tv_sec = 2.5;
  t.tv_usec = 0;

  // wait to receive, or for a timeout
  unsigned char buff[65536];
  int len = 65536;
  if (select(sock + 1, &socks, NULL, NULL, &t)) {
    if (recvfrom(sock, buff, len, 0, (struct sockaddr*)&in, &in_len) < 0) {
      printf("NORESPONSE\n");
      return -8;
    }
  } else {
    printf("NORESPONSE\n");
    return -8;
  }

  // Check for a valid response
  if (len == 0) {
    printf("NORESPONSE\n");
    return -9;
  } else if (len > 65536) {
    printf("ERROR\tLength of buffer is Beyond the limit.\n");
    return -9;
  }

  // Reader the Header into recieved header
  dns_header rec_header;
  int offset = read_header(buff, len, &rec_header);
  if (check_error()) {
    return -10;
  } else if (offset != 12) {
    printf("ERROR\tWhen Reading a Header.\n");
    return -10;
  }

  // Check the header ID
  if (!check_resp_header(&rec_header)) {
    if (check_error()) {
      return -11;
    }
    printf("ERROR\tHeader is Invalid.\n");
    return -11;
  }

  // Check Not Found
  if (rec_header.ancount == 0) {
    printf("NOTFOUND\n");
    return -13;
  }

  // Read the query
  dns_query rec_query;
  char* qname = read_query(buff, len, &offset, &rec_query);
  if (check_error()) {
    return -14;
  } else if (qname == NULL) {
    printf("ERROR\tInvalid QName within Response.\n");
    return -14;
  }

  // Check the Query
  if (rec_query.qtype_blank != query.qtype_blank || rec_query.qtype != query.qtype
    || rec_query.qclass_blank != query.qclass_blank || rec_query.qclass != query.qclass
    || strcmp(qname, target) != 0) {
    printf("ERROR\tInvalid Query in the Response\n");
    return -14;
  }

  // Iterate through all the answer
  for (unsigned int i = 0; i < rec_header.ancount; i++) {
    dns_response response;
    offset = read_response(buff, len, offset, &response);
    if (check_error()) {
      return -15;
    } else if (offset < 0) {
      printf("ERROR\tWhen Reading a Response.\n");
      return -15;
    } else {
      printf_response(&response, rec_header.aa);
    }
  }

  return 0;
}
