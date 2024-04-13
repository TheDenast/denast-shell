// Simple shell originally written for CS3650, NU, Fall 2023
// by Denast

// TODO: Everything except piping is finished

// Idk what this is but makes getline() work for me
#define _POSIX_C_SOURCE 200809L

// Used to quickly change input size (in chars)
#define INPUT_SIZE 256

#include <unistd.h>

#include <asm-generic/errno-base.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

// Function that takes an input string and tokenizes it into
// an array of string tokens
char **tokenize(char *input);

// Function that executes an array of tokens
int execute(char **tokens);

// Function that parses an array of tokens and
// calls execute() when necessary
int parse_exec(char **tokens);

// Debug function that goes through token array and prints
// them all
void print_tokens(char **tokens);

// Main function that contains the shell loop
int main(int argc, char **argv) {

  bool exit = false; // Used to quit main shell loop

  printf("Welcome to denast-shell :O\n");

  char *input = NULL;
  char **tokens = NULL;
  size_t input_size = INPUT_SIZE;
  size_t chars;

  input = (char *)malloc(input_size * sizeof(char));
  if (input == NULL) {
    perror("Unable to allocate buffer\n");
    return 1;
  }

  while (!exit) {
    printf("dnst $ ");
    chars = getline(&input, &input_size, stdin);

    if (strcmp("exit\n", input) == 0 || feof(stdin)) {
      // if (strcmp("exit\n", input) == 0) {
      printf("Bye bye.");
      exit = true;
      continue;
    } else if (strcmp("prev\n", input) == 0) {
      if (tokens != 0) {
        int x = 0;
        while (tokens[x] != NULL) {
          printf("%s ", tokens[x]);
          ++x;
        }
        printf("\n");
        parse_exec(tokens);
      } else {
        printf("No previous command\n");
        continue;
      }
    } else {

      // Call the tokenizer and receive token array
      tokens = tokenize(input);

      // Parse the array of tokens
      parse_exec(tokens);
    }
  }
  free(input);
  free(tokens);
  return 0;
}

int execute(char **tokens) {

  // Parse array for redirection symbol
  int x = 0;
  int redir = -1;
  char *dir = NULL;
  int fd = -1;
  dir = (char *)malloc(INPUT_SIZE * sizeof(char));
  int direction;

  while (tokens[x] != NULL) {
    if (strcmp(tokens[x], ">") == 0 || strcmp(tokens[x], "<") == 0) {
      redir = x;
      dir = tokens[x + 1];
      if (strcmp(tokens[x], ">") == 0) {
        fd = open(dir, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        direction = 1;
      } else {
        fd = open(dir, O_RDONLY);
        direction = 0;
      }
    }
    ++x;
  }

  // Create a fork to run command
  pid_t pid = fork();

  // If within child, execute/report error
  if (pid == 0) {

    if (redir != -1) {
      tokens[redir] = NULL;
      if (fd == -1) {
        printf("Error opening file\n");
        return 1;
      }
      dup2(fd, direction);
      close(fd);
      free(dir);
    }

    int errseq = execvp(tokens[0], tokens);
    if (errseq == -1) {
      printf("%s: command not found\n", tokens[0]);
    }
    _exit(0);
  } else { // In parent, wait for child
    waitpid(pid, 0, 0);
  }
  return 0;
}

int parse_exec(char **tokens) {
  // These two are used for parsing token array for ";"
  int pos1 = 0; // first token of current command
  int pos2 = 0; // last token of current command
  int x = 0;    // pos in parsing loop

  while (tokens[x] != NULL) {
    if (strcmp(tokens[x], ";") == 0 || tokens[x + 1] == NULL) { // Sequencing

      // Prepare a slice of token array to execute
      char **sliced_tokens = malloc(INPUT_SIZE * sizeof(char *));

      // This mumbo-jumbo is needed to differentiate between
      // Having ";" in the end of current slice and being at
      // the end of the entire token array;
      if (tokens[x + 1] == NULL)
        pos2 = x;
      else
        pos2 = x - 1;

      // Slicin
      for (int i = 0; i <= (pos2 - pos1); i++) {
        sliced_tokens[i] = tokens[i + pos1];
      }
      // Add null in the end of slice
      sliced_tokens[pos2 + 1] = NULL;

      pos1 = x + 1; // Realign starting position further into the array
                    // it has to be +1 since we want to skip ";"

      // Following section executes built-in commands
      // "cd" command
      if (strcmp(sliced_tokens[0], "cd") == 0) {
        const char *newDir = sliced_tokens[1];
        if (chdir(newDir) != 0) {
          printf("This directory does not exist\n");
        }
      }
      // "source command" (execute scripts)
      else if (strcmp(sliced_tokens[0], "source") == 0) {
        // attempt opening specified file
        FILE *script = fopen(sliced_tokens[1], "r");
        // if does not exist, print error, else execute
        if (script == NULL) {
          printf("Can't open file\n");
        } else {
          char *file_line = NULL;
          size_t file_line_len = INPUT_SIZE;
          size_t file_line_chars;
          file_line = (char *)malloc(file_line_len * sizeof(char));
          char **file_line_tokens = malloc(INPUT_SIZE * sizeof(char *));

          while ((file_line_chars =
                      getline(&file_line, &file_line_len, script)) != -1) {
            file_line_tokens = tokenize(file_line);
            parse_exec(file_line_tokens);
          }
          free(file_line);
          free(file_line_tokens);
          fclose(script);
        }

      }
      // "help" command (printing help info)
      else if (strcmp(sliced_tokens[0], "help") == 0) {
        printf("available built-ins:\nexit\texit shell\ncd\tchange "
               "directory\nsource\texecute "
               "script\nprev\texecute previous command\nhelp\tdisplay this "
               "message\n");
      }

      // If not one of built-ins, attempt execution
      else {
        execute(sliced_tokens);
      }
      // Don't forget to free memory used for slicing
      free(sliced_tokens);
    } else if (strcmp(tokens[x], "|") == 0) { // Piping
      printf("Oh no! There is a pipe but it is not implemented! *sadge* \n");
    }
    ++x;
    ++pos2;
  }
  return 0;
}

char **tokenize(char *expr) {

  // Since we can theoretically have max size input that
  // consists of 1-char tokens, reserve an array of that much strings
  // and then dynamically allocate memory for each token as they come
  char **tokens = malloc(INPUT_SIZE * sizeof(char *));

  int i = 0;            // position in input string
  int token_count = 0;  // position in token array
  int token_size = 0;   // used to reserve memory on per-token basis
  char token_buff[256]; // used to assemble token before writing into array

  while (expr[i] != '\0') { // while the end of string is not reached
    switch (expr[i]) {
    case '(':
      tokens[token_count] = malloc(1 * sizeof(char));
      tokens[token_count] = "(";
      break;
    case ')':
      tokens[token_count] = malloc(1 * sizeof(char));
      tokens[token_count] = ")";
      break;
    case '>':
      tokens[token_count] = malloc(1 * sizeof(char));
      tokens[token_count] = ">";
      break;
    case '<':
      tokens[token_count] = malloc(1 * sizeof(char));
      tokens[token_count] = "<";
      break;
    case ';':
      tokens[token_count] = malloc(1 * sizeof(char));
      tokens[token_count] = ";";
      break;
    case '|':
      tokens[token_count] = malloc(1 * sizeof(char));
      tokens[token_count] = "|";
      break;
    case '"': // Reading strings in quotes
      ++i;
      token_size = 0;

      // Run a loop until next " and populate token buffer
      while (expr[i] != '"' && expr[i] != '\n') {
        // printf("%c", expr[i]);
        token_buff[token_size] = expr[i];
        ++token_size;
        ++i;
      }
      // Dynamically allocate mem for current token depending
      // on its size
      tokens[token_count] = malloc((token_size + 1) * sizeof(char));

      // Transfer token from buffer to the array
      for (int k = 0; k < token_size; k++) {
        tokens[token_count][k] = token_buff[k];
      }
      if (expr[i] == '\0') {
        continue;
      }
      break;
    case ' ':        // Skip spaces
      --token_count; // Janky way to counter ++token_count since it is not a
                     // token
      break;
    default: // Reading Words
      token_size = 0;
      while (expr[i] != ' ' && expr[i] != '\n') {

        if (expr[i] == '(' || expr[i] == ')' || expr[i] == '<' ||
            expr[i] == '>' || expr[i] == '|' || expr[i] == ';') {
          --i;
          break;
        }

        token_buff[token_size] = expr[i];
        ++token_size;
        ++i;
      }
      // Dynamically allocate mem for current token depending
      // on its size
      tokens[token_count] = malloc((token_size + 1) * sizeof(char));

      // Transfer token from buffer to the array
      for (int k = 0; k < token_size; k++) {
        tokens[token_count][k] = token_buff[k];
      }
      if (expr[i] == '\0') {
        continue;
      }
    }
    ++i;           // advance to the next character
    ++token_count; // advance within token array
  }
  tokens[token_count] = NULL;
  return tokens;
}

void print_tokens(char **tokens) {
  int x = 0;
  while (tokens[x] != NULL) {
    printf("%s ", tokens[x]);
    ++x;
  }
  printf("\n");
}
