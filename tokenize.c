/**
 * A simple arithmetic expression tokenizer. Use this example to get you
 * started on the first task.
 */
#include <asm-generic/errno-base.h>
#include <stdio.h>

/**
 * Is the given character a digit?
 */
int is_digit(char ch) {
  // this relies on the fact that digits are ordered in the ASCII table
  return ch >= '0' && ch <= '9';
}

/**
 * Read the next integer as a string from the input into the output.
 */
int read_integer_string(const char *input, char *output) {
  int i = 0;
  // while we have input and the character is a digit,
  while (input[i] != '\0' && is_digit(input[i])) {
    output[i] = input[i]; // copy character to output buffer
    ++i;
  }
  output[i] = '\0'; // add the terminating byte

  return i; // return the length of the string
}

int main(int argc, char **argv) {

  char expr[256];
  fgets(expr, 256, stdin);

  char buf[256]; // temp buffer

  int i = 0;                // current position in string
  while (expr[i] != '\0') { // while the end of string is not reached
    // first check if the current char is a digit
    // if (is_digit(expr[i])) {
    //   // read the integer from the output AND
    //   // advance the current position by the length of the string
    //   i += read_integer_string(&expr[i], buf);
    //   // could convert it here...
    //   printf("%s\n", buf);
    //   continue; // skip the rest of this iteration
    // }

    // if not number, consider the current character and print its type
    switch (expr[i]) {
    case '(':
      printf("(\n");
      break;
    case ')':
      printf(")\n");
      break;
    case '>':
      printf(">\n");
      break;
    case '<':
      printf("<\n");
      break;
    case ';':
      printf(";\n");
      break;
    case '|':
      printf("|\n");
      break;
    case '"':
      // Reading strings in quotes
      ++i;
      while (expr[i] != '"' && expr[i] != '\0') {
        printf("%c", expr[i]);
        ++i;
      }
      if (expr[i] != '\0') {
        printf("\n");
      }
      break;
    case ' ':
      // skip spaces
      break;
    default:
      // Reading words
      while (expr[i] != ' ' && expr[i] != '\0') {
        printf("%c", expr[i]);
        ++i;
      }
      if (expr[i] != '\0') {
        printf("\n");
      } else {
        continue;
      }
    }
    ++i; // advance to the next character
  }

  return 0;
}
