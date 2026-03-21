#include <stdio.h>
#include "ctype.h"
#include <string.h>
#include <stdbool.h>

// terminal tokens
typedef enum {read, write, id, literal, becomes,
                add, sub, mul, div, lparen, rparen, eof} token;

char token_image[100];

// terminal names
char* names[] = {"read", "write", "id", "literal", "becomes",
                "add", "sub", "mul", "div", "lparen", "rparen", "eof"};

extern void exit(int);

char token_char;

static token input_token; // parser lookahead token

int line = 1;

void error(int c){
    if (c == EOF){
        printf( "Lexical error at line %d - Unexpected EOF\n", line);
    } else {
        printf( "Lexical error at line %d - Unexpected token: %c\n", line, c);
    }
    exit(1);
}

void error_unterminated_comment() {
    printf("Lexical error at line %d - Unterminated block comment\n", line);
    exit(1);
}

token scan(FILE *file) {
    static int c = ' ';
        /* next available char; extra (int) width accommodates EOF */
    int i = 0;              /* index into token_image */

    if (c == '\n') {
        line++;
    }

    /* skip white space */
    while (isspace(c)) {
        c = fgetc(file);
        token_char = c;
    }
    if (c == EOF)
        return eof;
    if (isalpha(c)) {
        /* handle identifiers and reserved words */
        do {
            token_image[i++] = c;
            c = fgetc(file);
        } while (isalpha(c) || isdigit(c) || c == '_');
        token_image[i] = '\0';
        if (!strcmp(token_image, "read")) return read;
        else if (!strcmp(token_image, "write")) return write;
        else return id;
    }
    else if (isdigit(c)) {
        /* handle integer literals */
        do {
            token_image[i++] = c;
            c = fgetc(file);
        } while (isdigit(c));
        token_image[i] = '\0';
        return literal;
    } else switch (c) {
        case ':':
            c = fgetc(file);
            /* handle assignment operator */
            if (c != '=') {
                error(c);
            } else {
                c = fgetc(file);
                return becomes;
            }
        case '+': c = fgetc(file); return add;
        case '-': c = fgetc(file); return sub;
        case '*': c = fgetc(file); return mul;
        case '/':
            c = fgetc(file); 
            if (c == '/') {
                /* skip until end of line comment */
                while (c != '\n' && c != EOF) {
                    c = fgetc(file);
                }
                /* recurse to continue scanning */
                return scan(file);
            } else if (c == '*') {
                bool isComment = false;
                /* scan until closing */ 
                while (1) {
                    c = fgetc(file);
                    if (c == '*') {
                        c = fgetc(file);
                        if (c == '/') {
                            isComment = true;
                            break;
                        }
                        if (c == EOF) {
                            error_unterminated_comment();
                        }
                    }
                    if (c == EOF) {
                        error_unterminated_comment();
                    }
                }
                if (isComment){
                    c = fgetc(file);
                    /* recurse to continue scanning */
                    return scan(file);
                }
                break;
            } else {
                /* handle division operator */
                return div;
            }
        case '(': c = fgetc(file); return lparen;
        case ')': c = fgetc(file); return rparen;
        default:
            error(c);
    }

    return eof;
}

