/*
 * CS3600, Spring 2013
 * Project 1 Starter Code
 * (c) 2013 Alan Mislove
 *
 * You should use this (very simple) starter code as a basis for
 * building your shell.  Please see the project handout for more
 * details.
 */

#ifndef _3600sh_h
#define _3600sh_h

#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

// Standard stream constants
const int STDIN = 0;
const int STDOUT = 1;
const int STDERR = 2;

// Boolean values by name
typedef int bool;
enum { false, true };

// Struct that stores a parsed input value
typedef struct parsed_s {
  // The name of the command to be run
  char* cmd;
  // The number or arguments passed in addition to the command
  unsigned int argc;
  // The tokenized additional arguments
  char** argv;
  // Is this to be processed in the background?
  bool background;
  // Is this the exit call?
  bool exit;
  // Did this end with the EOF command?
  bool end_of_file;
  // Was there a stdin redirect?
  bool in_redirect;
  // Was there a stdout redirect?
  bool out_redirect;
  // Was there a stderr redirect?
  bool err_redirect;
  // The name of the stdin redirect (or NULL)
  char* in_name;
  // The name of the stdout redirect (or NULL)
  char* out_name;
  // The name of the stderr redirect (or NULL)
  char* err_name;
} parsed;

// Free the given input and parsed-frees the given parsed 
//  and check for end of file, if one is found exit the shell
void free_and_check_eof2(char* input, parsed* prsd);

// Parsed-frees the given parsed 
//  and check for end of file, if one is found exit the shell
void free_and_check_eof(parsed* prsd);

// Gets the next line of input from the prompt
char* get_input_line(parsed *prsd);

// Parse the given input string into a valid form into an array of strings.
//  The array is guaranteed to end with a NULL pointer in the last position.
//  Return a NULL if the input was not valid.
char** parse(char* input);

// Get the size of the str array with a NULL in the last position
int size_of_str_array(char** strs);

// Convert the given array of strings into the given parsed.
// Return 0 if program complete
int strings_to_parsed(char** strs, parsed* prsd);

// Execute the given command struct
int run(parsed* input);

// Exit the shell
void do_exit();

// Function which frees the given parsed
//  then exits, printing the necessary message
void do_exit_and_free(parsed *prsd);

// Free the given parsed and its subsequent variables
void free_parsed(parsed *prsd);

// Extend the given string from the given length to a length +
//  the given extension. Should include the null character
//  and generates a null character at the end of the new string
char* extend_str(char* str, int* len, int extension);

// Extend the given array of strings ending with a NULL pointer
//  from the given len to the that plus the given extension and
//  end it with a NULL 
char** extend_str_array(char** strs, int *len, int extension) ;


// Test the size_of_str_array function
void test_sosa();

// Test the central functions of the#include <assert.h> program
void test_parsing();

// Feed the given string into stdin and print the location of the previous standard in
void feed_to_stdin(char* str);

// Are the given arrays of string terminating with a null pointer in the final
//  position equal?
bool equals_strs(char** actual, char** expected);

// Are the given parsed equal?
bool equals_parsed(parsed actual, parsed expected);

// Are the given bools representing whether the cstring are active are equal
//  and if they are both true then return whether the cstrings are equal
bool equals_bool_str(bool actual_active, char* actual, 
  bool expected_active, char* expected);

#endif 
