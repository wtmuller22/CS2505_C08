#include <stdint.h>
#include <stdio.h>
#include "StringHashTable.h"
// On my honor:
// // //
// // // - I have not discussed the C language code in my program with
// // // anyone other than my instructor or the teaching assistants
// // // assigned to this course.
// // //
// // // - I have not used C language code obtained from another student,
// // // the Internet, or any other unauthorized source, either modified
// // // or unmodified.
// // //
// // // - If any C language code or documentation used in my program
// // // was obtained from an authorized source, such as a text book or
// // // course notes, that has been clearly noted with a proper citation
// // // in the comments of my program.
// // //
// // // - I have not designed this program in such a way as to defeat or
// // // interfere with the normal operation of the Curator System.
// // //
// // // Wyatt Muller
// // // wtmuller22

struct _FIDObject {
	int fid;
	uint32_t offset;
};
typedef struct _FIDObject FIDObject;

FIDObject* FIDObject_create(int id, uint32_t loc);

uint32_t continue_until(FILE* file, char end);

uint32_t elfhash(const char* str);

uint32_t fill_indicies(FILE* file, FIDObject** array, StringHashTable* table, uint32_t array_size);

int compare(const void *left, const void *right);

void parse_commands(FILE* in, FILE* out, FILE* data, FIDObject* array, StringHashTable* table, uint32_t numEle);

int array_length(uint32_t* array);

int compare_offsets(const void* left, const void* right);

char* separate_coord(char* coord, int lalo);
