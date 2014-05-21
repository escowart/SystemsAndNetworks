/**
 * CS3600, Spring 2013
 * Project 1 Starter Code
 * (c) 2013 Alan Mislove
 *
 * You should use this (very simple) starter code as a basis for 
 * building your shell.  Please see the project handout for more
 * details.
 */

/*
 * Completed by
 * Team MuffinButton
 * (Cody Wetherby / Edwin Cowart)
 * on 2014/01/30
 */

#include "3600sh.h"

#define USE(x) (x) = (x)

char* cwd;
char user[200];
char host[200];

int main(int argc, char*argv[]) {
  unsigned int buffer = 200;
  cwd = (char*) calloc(buffer, sizeof(char));
  getcwd(cwd, buffer);
  strncpy(user, getenv("USER"), buffer);
  gethostname(host, buffer);

  // Code which sets stdout to be unbuffered:
  // This is necessary for testing; do not change these lines
  USE(argc);
  USE(argv);
  setvbuf(stdout, NULL, _IONBF, 0);

  if (argc == 2 && strcmp(argv[1], "-unittest") == 0) {
    test_sosa();
    test_parsing();
    printf("All Test Passed\n");
  }
  
  // Main loop that reads a command and executes it
  while (1) {
    // Print prompt
    printf("%s@%s:%s> ", user, host, cwd);
    parsed prsd = {NULL, 0, NULL, false, false, false, false, false, false, 
      NULL, NULL, NULL};
    char* input = get_input_line(&prsd);
    // If input was empty
    if (strlen(input) == 0) {
      free_and_check_eof2(input, &prsd);
      continue;
    }
    char** strs = parse(input);
    if (strs == NULL) {
      printf("Error: Unrecognized escape sequence.\n");
      free_and_check_eof2(input, &prsd);
      continue;
    }
    else if (strs[0] == NULL) {
      free_and_check_eof2(input, &prsd);
      continue;
    }
    /**
    printf("strs:\n");
    for (int index = 0; index < size_of_str_array(strs); index++) {
      printf("\t%s\n", strs[index]);
    }
    */
    int stp = strings_to_parsed(strs, &prsd);
    if (stp < 0) {
      printf("Error: Invalid syntax.\n");
      free_and_check_eof2(input, &prsd);
      continue;
    }
    free(input);
    /**
    printf("parsed:\n");
    printf("\tcmd: %s\n", prsd.cmd);
    printf("\targc: %i\n", prsd.argc);
    printf("\targv:\n");
    for (unsigned int i = 0; i < prsd.argc; i++)
      printf("\t\t[%i]: %s\n", i, prsd.argv[i]);
    printf("\tbackground: %s\n", (prsd.background ? "true" : "false"));
    printf("\texit: %s\n", (prsd.exit ? "true" : "false"));
    printf("\tend_of_file: %s\n", (prsd.end_of_file ? "true" : "false"));
    printf("\tin_redirect: %s\n", (prsd.in_redirect ? "true" : "false"));
    printf("\tout_redirect: %s\n", (prsd.out_redirect ? "true" : "false"));
    printf("\terr_redirect: %s\n", (prsd.err_redirect ? "true" : "false"));
    printf("\tin_name: %s\n", prsd.in_name);
    printf("\tout_name: %s\n", prsd.out_name);
    printf("\terr_name: %s\n", prsd.err_name);
    */
    run(&prsd);
    free_and_check_eof(&prsd);
  }

  return 0;
}

// Free the given input and parsed-frees the given parsed 
//  and check for end of file, if one is found exit the shell
void free_and_check_eof2(char* input, parsed* prsd) {
  free(input);
  free_and_check_eof(prsd);
}

// Parsed-frees the given parsed 
//  and check for end of file, if one is found exit the shell
void free_and_check_eof(parsed* prsd) {
  free_parsed(prsd);
  if (prsd->end_of_file) {
    do_exit();
  }
}


// Gets the next line of input from the prompt
char* get_input_line(parsed *prsd) {
  char next;
  unsigned int size = 500;
  char* ans = (char*) calloc(size, sizeof(char));
  bool done = false;
  unsigned int index = 0;
  do {
    if (index+2 >= size) {
      char* temp = calloc(size, sizeof(char));
      strncpy(temp, ans, size);
      size += 100;
      ans = (char*) realloc(ans, size*sizeof(char));
      strcpy(ans, temp);
      free(temp);
    }
    next = getc(stdin);
    if (next == '\n') {
      ans[index] = '\0';
      done = true;
    } else if (next == EOF) {
      // End of File: Change the parsed field
      prsd->end_of_file = true;
      ans[index] = '\0';
      done = true;
    } else {
      ans[index++] = next;
    }
  }
  while (!done);

  return ans;
}

// Parse the given input string into a valid form into an array of strings.
//  Change the EOF value state of the given parsed to true if EOF found.
//  The array is guaranteed to end with a NULL pointer in the last position.
//  Return a NULL if the input was not valid.
char** parse(char* input) {
  // The acc size
  unsigned int acc_size = 50;
  // The next index to get a new element
  unsigned int acc_idx = 0;
  // The array of strings of the input parsed
  char** acc = (char**) calloc(acc_size, sizeof(char*));
  acc[acc_size - 1] = NULL;
  // The size of the current string
  unsigned int str_size = 200;
  // The index to the next char to be added to the string
  unsigned int str_idx = 0;
  // The current string
  char* str;
  // The next character in the input
  char next;
  // Is the previous char is a normal char or an escape character?
  bool prevnorm = false;
  do {
    // The next character in the input
    next = *(input++);
    if (next == '\0') {
      // End of input: break
      break;
    } else if (next == '&' || next == '<' || next == '>' ) {
      // & or < or > ends the string before it and then adds 
      // itself as a new string.
      // If the previous string has not been set then an empty string
      //  should not be created otherwise end the previous string.
      if (str_idx != 0) {
        str[str_idx] = '\0';
        str_idx = 0;
      }
      // Make the & into its own string
      str = (char*) calloc(2, sizeof(char));
      str[0] = next;
      str[1] = '\0';
      acc[acc_idx++] = str;
      prevnorm = false;
    } else if (next == '2' && *input == '>') {
      // 2> ends the string before it and then adds 
      // itself as a new string.
      // If the previous string has not been set then an empty string
      //  should not be created otherwise end the previous string.
      if (str_idx != 0) {
        str[str_idx] = '\0';
        str_idx = 0;
      }
      // Make the & into its own string
      str = (char*) calloc(3, sizeof(char));
      str[0] = next;
      next = *(input++);
      str[1] = next;
      str[2] = '\0';
      acc[acc_idx++] = str;
      prevnorm = false;
    } else if (next == '\\') {
      if (str_idx == 0) {
        str = (char*) calloc(str_size, sizeof(char));
        str[str_size - 1] = '\0';
        acc[acc_idx++] = str;
      }
      // The escape character indicates that the next character is read
      //  in a special way.
      // next now points to the next-next character
      next = *(input++);
      if (next == 't') {
        // A "\t" becomes '\t'
        str[str_idx++] = '\t';
      } else if (next == ' ' || next == '\\' || next == '&') {
        // A "\ " becomes ' ', a "\\" becomes '\', 
        //  and a "\&" becomes '&'
        str[str_idx++] = next;
      } else {
        // Everything else throws an Error message
        unsigned int i = 0;
        for (; i < acc_idx; i++) {
          free(acc[i]);
        }
        free(acc);
        return NULL;
      }
      prevnorm = true;
    } else if (next == ' ' || next == '\t') {
      // White space should be ignored unless the current string      
      //  needs to ended.
      if (str_idx != 0) {
        str[str_idx++] = '\0';
        str_idx = 0;
      }
      prevnorm = false;
    } else {
      // If str has been reset then calloc a new one
      if (str_idx == 0) {
        str = (char*) calloc(str_size, sizeof(char));
        str[str_size - 1] = '\0';
        acc[acc_idx++] = str;
      }
      // Add this char as the next element in the str
      str[str_idx++] = next;
      prevnorm = true;
    }
    if (acc_idx >= acc_size - 3 || str_idx >= str_size - 2) {
      break;
    }
  } while (true);
  if (prevnorm) {
    str[str_idx] = '\0';
  }
  // The last element must be a NULL
  acc[acc_idx] = NULL;
  // return the parse version of the input string
  return acc;
}


// Get the size of the str array with a NULL in the last position
int size_of_str_array(char** strs) {
  int i = 0;
  while (*strs != NULL) {
    i++;
    strs++;
  }
  return i;
}

// Convert the given array of strings into the given parsed.
// Return 0 if program complete
int strings_to_parsed(char** strs, parsed* prsd) {
  // The size of strs
  unsigned int size = size_of_str_array(strs);

  // The current index of strs whose element is being operated on
  unsigned int i = 0;

  // Has the cmd been set yet?                                            
  bool cmd_set = false;
 
  // 0 = input redirect not yet set
  // 1 = next string is the file for the redirect
  // 2 = input redirect set
  int in_redir_set = 0;

  // 0 = output redirect not yet set
  // 1 = next string is the file for the redirect
  // 2 = output redirect set
  int out_redir_set = 0;
	
  // 0 = error redirect not yet set
  // 1 = next string is the file for the redirect
  // 2 = error redirect set
  int err_redir_set = 0;
	
  // Is the given statement, strs, bad?
  bool bad = false;
	
  // The args size
  unsigned int args_size = 50;
  // The array of strings of the arguments ending with a NULL
  char** args = (char**) calloc(args_size, sizeof(char*));
  args[args_size - 1] = NULL;
  prsd->argv = args;
  prsd->argc = 0;

  // The current str 
  char *s;
  // loop until every element in strs is visited
  for (i = 0; i < size; i++) {
    // The current element
    s = strs[i];
    if (strcmp(s, "&") == 0) {
      // The "&" must be at the end of strs and not at the beginning
      s = strs[++i];
      // If one can do a valid run in background
      if (s == NULL && cmd_set && in_redir_set != 1 
           && out_redir_set != 1 && err_redir_set != 1) {
        prsd->background = true;
      } else {
       // Else the program should terminate improperly
       bad = true;
      }
      break;
    } else if (strcmp(s, "<") == 0) {
      // Do input redirect if able else set bad
      if (cmd_set && in_redir_set == 0 && out_redir_set != 1
           && err_redir_set != 1) {
        prsd->in_redirect = true;
        in_redir_set++;
      } else {
        bad = true;
        break;
      }
    } else if (strcmp(s, ">") == 0) {
      // Do output redirect if able else set bad
      if (cmd_set && in_redir_set != 1 && out_redir_set == 0 
          && err_redir_set != 1) {
        prsd->out_redirect = true;
        out_redir_set++;
      } else {
        bad = true;
        break;
      }
    } else if (strcmp(s, "2>") == 0) {
      // Do error redirect if able else set bad
      if (cmd_set && in_redir_set != 1 && out_redir_set != 1
           && err_redir_set == 0) {
        prsd->err_redirect = true;
        err_redir_set++;
      } else {
        bad = true;
        break;
      }
    } else {
      if (!cmd_set) {
        // Set the cmd
        prsd->cmd = (char*) calloc(strlen(s) + 1, sizeof(char));
        strcpy(prsd->cmd, s);
        cmd_set = true;
        if (strcmp(s, "exit") == 0) {
          prsd->exit = true;
        }
        // Add the command to the front of argv for execvp
        args[prsd->argc++] = prsd->cmd;
      } else if (in_redir_set == 1) {
        // Set the file for input redirect
        prsd->in_name = (char*) calloc(strlen(s) + 1, sizeof(char));
        strcpy(prsd->in_name, s);
        in_redir_set++;
      } else if (out_redir_set == 1) {
        // Set the file for output redirect
        prsd->out_name = (char*) calloc(strlen(s) + 1, sizeof(char));
        strcpy(prsd->out_name, s);
        out_redir_set++;
      } else if (err_redir_set == 1) {
        // Set the file for error redirect
        prsd->err_name = (char*) calloc(strlen(s) + 1, sizeof(char));
        strcpy(prsd->err_name, s);
        err_redir_set++;
      } else {
        if (prsd->argc >= args_size - 1) {
          args[prsd->argc] = NULL;
          continue;
        }
        // Add the new element to the acc
        args[prsd->argc] = (char*) calloc(strlen(s) + 1, sizeof(char));
        strcpy(args[prsd->argc++], s);
      }
    }
  }
  free(strs);
  return -1 * (bad || !cmd_set || in_redir_set == 1 || 
    out_redir_set == 1 || err_redir_set == 1);
}

// Execute the given command struct
int run(parsed* input) {
  if (input->exit) {
    do_exit_and_free(input);
  }
  pid_t pid = fork();
  int stat = 0;
  if (pid < 0) {
    printf("Error: Fork Failed.");
  } else if (pid == 0) {
    if (input->in_redirect) {
      int file_in = open(input->in_name, O_RDONLY);
      if (file_in < 0) {
        if (errno == ENOENT) {
          printf("Error: Unable to open redirection file.\n");
        } else {
          printf("Error: input redirect: %s.\n", strerror(errno));
        }
        exit(-1);
      }
      int in_result = dup2(file_in, STDIN);
      if (in_result < 0) {
        printf("Error: %s\n", strerror(errno));
        exit(-2);
      }
      close(file_in);
    }
    if (input->out_redirect) {
      int file_out = open(input->out_name, O_TRUNC ^ O_WRONLY ^ O_CREAT, 
        S_IRUSR ^ S_IWUSR ^ S_IRGRP ^ S_IROTH);
      if (file_out < 0) {
        if (errno == ENOENT) {
          printf("Error: Unable to open redirection file.\n");
        } else {
          printf("Error: output redirect: %s.\n", strerror(errno));
        }
        exit(-1);
      }
      int out_result = dup2(file_out, STDOUT);
      if (out_result < 0) {
        printf("Error: error redirect: %s.\n", strerror(errno));
        exit(-2);
      }
      close(file_out);
    }
    if (input->err_redirect) {
      int file_err = open(input->err_name, O_TRUNC ^ O_WRONLY ^ O_CREAT, 
        S_IRUSR ^ S_IWUSR ^ S_IRGRP ^ S_IROTH);
      if (file_err < 0) {
        if (errno == ENOENT) {
          printf("Error: Unable to open redirection file.\n");
        } else {
          printf("Error: error redirect: %s.\n", strerror(errno));
        }
        exit(-1);
      }
      int err_result = dup2(file_err, STDERR);
      if (err_result < 0) {
        if (errno == ENOENT) {
          printf("Error: %s.\n", strerror(errno));
        }
        exit(-2);
      }
      close(file_err);
    }
    execvp(input->cmd, input->argv);

    // This code below is never run unless exec has an error
    if (errno == EACCES) {
      printf("Error: Permission denied.\n");
    } else if (errno == ENOENT) {
      printf("Error: Command not found.\n");
    } else {
      printf("Error: %s.\n", strerror(errno));
    }
    exit(-1);
  } else {
    if (!input->background) {
      waitpid(pid, &stat, 0);
    }
  }
  return stat;
}

// Function which exits, printing the necessary message
void do_exit() {
  free(cwd);
  printf("So long and thanks for all the fish!\n");
  exit(0);
}

// Function which frees the given parsed
//  then exits, printing the necessary message
void do_exit_and_free(parsed *prsd) {
  free_parsed(prsd);
  do_exit();
}

// Free the given parsed and its subsequent variables
void free_parsed(parsed *prsd) {
  if (prsd->cmd != NULL) {
    free(prsd->cmd);
  }
  if (prsd->argv != NULL) {
    while (*(prsd->argv++) != NULL) {
      free(*prsd->argv);
    }
  }
  if (prsd->in_name != NULL) {
    free(prsd->in_name);
  }
  if (prsd->out_name != NULL) {
    free(prsd->out_name);
  }
  if (prsd->err_name != NULL) {
    free(prsd->err_name);
  }
}


// Test the size_of_str_array function
void test_sosa() {
  char* strs0[] = {NULL};
  assert(size_of_str_array(strs0) == 0);
  char* strs1[] = {"hello", "world", NULL};
  assert(size_of_str_array(strs1) == 2);
}



// Test the central functions of the program
void test_parsing() {
  int num_of_tests = 50;
  char *strs[num_of_tests];
  int size = 0;
  strs[size++] = "ls";
  strs[size++] = "ls 	-l    -a	 	 &	 ";
  //strs[size++] = "echo hello\\ \\&\\\\tworld<text.txt>hello.txt 2>hello2.txt&";
  strs[size] = NULL;

  int i = 0;
  char **parse_results[num_of_tests];
  char* strs0[] = {"ls", NULL};
  parse_results[i++] = strs0;
  char* strs1[] = {"ls", "-l", "-a", "&", NULL};
  parse_results[i++] = strs1;
  //char* strs2[] = {"echo", "hello &\\	world", "<", "text.txt", ">", 
  //  "hello.txt", "2>", "hello2.txt", "&", NULL};
  //parse_results[i++] = strs2;
  
  i = 0;
  parsed stp_results[num_of_tests];
  parsed prsd0 = {strs[0], 1, strs0, false, false, false, false, false, 
    false, NULL, NULL, NULL};
  stp_results[i++] = prsd0;
  char* argv1[] = {"ls", "-l", "-a", NULL};
  parsed prsd1 = {strs[0], 3, argv1, true, false, false, false, false, 
    false, NULL, NULL, NULL};
  stp_results[i++] = prsd1;
  //char* argv2[] = {"echo", "hello &\\	world", NULL};
  //parsed prsd2 = {"echo", 2, argv2, true, false, false, true, true, 
  //  true, "text.txt", "hello.txt", "hello2.txt"};
  //stp_results[i++] = prsd2;

  for (i = 0; i < size; i++) {
    feed_to_stdin(strs[i]);

    parsed prsd = {NULL, 0, NULL, false, false, false, false, false, 
      false, NULL, NULL, NULL};
    // There is currently a bug in the get_input_line unit test
    //char* input = get_input_line(&prsd);
    //printf("actual: %s\nexcpected: %s\n", input, strs[i]);
    //assert(strcmp(input, strs[i]) == 0);
    char** sts = parse(strs[i]);
    assert(equals_strs(sts, parse_results[i]));
    if (sts == NULL || sts[0] == NULL) {
      //free(input);
      continue;
    }
    strings_to_parsed(sts, &prsd);
    assert(equals_parsed(prsd, stp_results[i]));
    //free(input);
  }
}

// Feed the given string into stdin and print the location of the previous standard in
void feed_to_stdin(char* str) {
  int temp = open("temp.txt", O_TRUNC ^ O_WRONLY ^ O_CREAT, 
        S_IRUSR ^ S_IWUSR ^ S_IRGRP ^ S_IROTH);
  int len = strlen(str);
  char s0[len + 3];
  strcpy(s0, str);
  s0[len] = '\n';
  s0[len + 1] = '\0';
  s0[len + 2] = EOF;
  write(temp, s0, 1000);
  close(temp);
  open("temp.txt", O_RDONLY);
  dup2(temp, STDIN);
  close(temp);
}

// Are the given arrays of string terminating with a null pointer in the final
//  position equal?
bool equals_strs(char** actual, char** expected) {
  if (actual == NULL) {
    return expected == NULL;
  } else {
    if (expected == NULL) {
      return false;
    } else {
      int size0 = size_of_str_array(actual);
      int size1 = size_of_str_array(expected);
      if (size0 == size1) {
        for (int i = 0; i < size0; i++) {
          if (strcmp(actual[i], expected[i]) != 0) {
            return false;
          }
        }
        return true;
      } else {
        return false;
      }
    }
  }
}

// Are the given bools representing whether the cstring are active are equal
//  and if they are both true then return whether the cstrings are equal
bool equals_bool_str(bool actual_active, char* actual, 
  bool expected_active, char* expected) {
  if (actual_active) {
    if (expected_active) {
      if (actual == NULL || expected == NULL) {
        return false;
      }
      return strcmp(actual, expected) == 0;
    } else {
      return false;
    }
  } else {
    return !expected_active;
  }
}

// Are the given parsed equal?
bool equals_parsed(parsed actual, parsed expected) {
  return (strcmp(actual.cmd, expected.cmd) == 0) &&
        actual.argc == expected.argc &&
        equals_strs(actual.argv, expected.argv) &&
        actual.background == expected.background &&
        actual.exit == expected.exit &&
        actual.end_of_file == expected.end_of_file &&
        equals_bool_str(actual.in_redirect, actual.in_name, 
          actual.in_redirect, actual.in_name) &&
        equals_bool_str(actual.out_redirect, actual.out_name, 
          actual.out_redirect, actual.out_name) &&
        equals_bool_str(actual.err_redirect, actual.err_name, 
          actual.err_redirect, actual.err_name);
}

