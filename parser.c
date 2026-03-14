/* 
    Table Driven Parser for the calculator language.
*/
#include <stdio.h>
#include <string.h>

typedef enum {read, write, id, literal, becomes,
                add, sub, mul, div, lparen, rparen, eof} token;

extern char token_image[];

extern char *names[];

const char *nt_names[] = {
    "program", "stmt_list", "stmt", "expr", "term_tail", 
    "term", "factor_tail", "factor", "add_op", "mult_op"
};

static token input_token;

static FILE *file;

token scan(FILE *file);

extern int line;
extern char token_char;
extern void exit(int);

int stack[100];
int stack_top = 0;

typedef enum { program=12, stmt_list, stmt, expr, term_tail, term, factor_tail, factor, add_op, mult_op } non_terminal;

int productions[100][100] = {
    {stmt_list, eof, -1}, // program
    {stmt, stmt_list, -1}, // stmt_list
    {-1}, // stmt_list (epsilon)
    {id, becomes, expr, -1}, // stmt (assignment)
    {read, id, -1}, // stmt (read)
    {write, expr, -1}, // stmt (write)
    {term, term_tail, -1}, // expr
    {add_op, term, term_tail, -1}, // term_tail (add)
    {-1}, // term_tail (epsilon)
    {factor, factor_tail, -1}, // term
    {mult_op, factor, factor_tail, -1}, // factor_tail (mul)
    {-1}, // factor_tail (epsilon)
    {lparen, expr, rparen, -1}, // factor (parenthesized expression)
    {id, -1}, // factor (identifier)
    {literal, -1}, // factor (literal)
    {add, -1}, // add_op (add)
    {sub, -1}, // add_op (sub)
    {mul, -1}, // mult_op (mul)
    {div, -1}, // mult_op (div)
};

int table[100][100];


void initialize_table() {
    // Initialize the table to -1
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            table[i][j] = -1;
        }
    }

    // Initialize the table with the productions
    table[program][id] = 1;
    table[program][read] = 1;
    table[program][write] = 1;
    table[program][eof] = 1;

    table[stmt_list][id] = 2;
    table[stmt_list][read] = 2;
    table[stmt_list][write] = 2;
    table[stmt_list][eof] = 3;

    table[stmt][id] = 4;
    table[stmt][read] = 5;
    table[stmt][write] = 6;

    table[expr][id] = 7;
    table[expr][literal] = 7;
    table[expr][lparen] = 7;

    table[term_tail][id] = 9;
    table[term_tail][read] = 9;
    table[term_tail][write] = 9;
    table[term_tail][rparen] = 9;
    table[term_tail][add] = 8;
    table[term_tail][sub] = 8;
    table[term_tail][eof] = 9;

    table[term][id] = 10;
    table[term][literal] = 10;
    table[term][lparen] = 10;

    table[factor_tail][id] = 12;
    table[factor_tail][read] = 12;
    table[factor_tail][write] = 12;
    table[factor_tail][rparen] = 12;
    table[factor_tail][add] = 12;
    table[factor_tail][sub] = 12;
    table[factor_tail][mul] = 11;
    table[factor_tail][div] = 11;
    table[factor_tail][eof] = 12;

    table[factor][id] = 14;
    table[factor][literal] = 15;
    table[factor][lparen] = 13;

    table[add_op][add] = 16;
    table[add_op][sub] = 17;

    table[mult_op][mul] = 18;
    table[mult_op][div] = 19;
}

void systax_error(int expected, int top){
    printf( "Syntax error at line %d. ", line);

    char found[100] = "";
    if (input_token == id || input_token == literal) {
        strcpy(found, token_image);
    } else {
        strcpy(found, names[input_token]);
    }

    if (expected != -1) {
        printf("Expected token: %s, found: %s\n", names[expected], found);
    }

    if (top != -1) {
        printf("Unexpected %s while parsing %s\n", found, nt_names[top - program] );
    }

    exit(1);
}

void match(token expected) {
    if (input_token == expected) {
    	printf ("Token matched: %s\n", names[input_token]);
        input_token = scan(file);
    }
    else systax_error(expected, -1);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("You must provide one argument (input file).");
        return 1;
    }

    file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("Error opening file");
        return 1;
    }

    initialize_table();

    input_token = scan(file);
    stack[stack_top] = program;
    stack_top++;

    while (stack_top > 0) {
        int top = stack[stack_top - 1];
        stack_top--;

        if (top < 12) {
            // Terminal
            match(top);
        } else {
            // Non-terminal
            int production = table[top][input_token];
            if (production == -1) {
                systax_error(-1, top);
            } else {
                int size = 0;
                while (productions[production - 1][size] != -1) {
                    size++;
                }

                while (size > 0) {
                    int prod_value = productions[production - 1][size - 1];
                    stack[stack_top] = prod_value;
                    stack_top++;
                    size--;
                }
            }
        }
    }

    printf("No lexical or syntax errors found.\n");
    fclose(file);
}
