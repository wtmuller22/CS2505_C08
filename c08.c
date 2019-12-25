#include "c08.h"
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>
#include <sys/types.h>
#include <math.h>
#define PI 3.14159265358979323846
#define RADIUS 6371.0088
// On my honor:
// //
// // - I have not discussed the C language code in my program with
// // anyone other than my instructor or the teaching assistants
// // assigned to this course.
// //
// // - I have not used C language code obtained from another student,
// // the Internet, or any other unauthorized source, either modified
// // or unmodified.
// //
// // - If any C language code or documentation used in my program
// // was obtained from an authorized source, such as a text book or
// // course notes, that has been clearly noted with a proper citation
// // in the comments of my program.
// //
// // - I have not designed this program in such a way as to defeat or
// // interfere with the normal operation of the Curator System.
// //
// // Wyatt Muller
// // wtmuller22

int main(int argc, char **argv) {
	FILE *In = fopen(argv[1], "r");
	if (In == NULL){
		printf("ERROR: Could not find command script.");
	}
	FILE *Out = fopen(argv[2], "w");
	while ((char)fgetc(In) == ';'){
		continue_until(In, '\n');
	}
	char* filename = malloc(50 * sizeof(char));
	fscanf(In, "%*s%s", filename);
	FILE *Data = fopen(filename, "r");
	free(filename);
	uint32_t table_size;
	fscanf(In, "%*s%"PRIu32"\n", &table_size);
	FIDObject *fidIdx = malloc(256 * sizeof(FIDObject));
	StringHashTable* table = StringHashTable_create(table_size, elfhash);
	uint32_t array_size = 256;
	uint32_t numEle = fill_indicies(Data, &fidIdx, table, array_size);
	qsort(fidIdx, numEle, sizeof(FIDObject), compare);
	parse_commands(In, Out, Data, fidIdx, table, numEle);	
	StringHashTable_clear(table);
	free(table);
	free(fidIdx);
	fclose(Data);
	fclose(In);
	fclose(Out);
	return 0;
}

FIDObject* FIDObject_create(int id, uint32_t loc){
        FIDObject* obj = (FIDObject*)malloc(sizeof(FIDObject));
        obj->fid = id;
        obj->offset = loc;
        return obj;
}

uint32_t continue_until(FILE* file, char end){
	uint32_t num_parsed = 1;
	char c = (char)fgetc(file);
	while (c != end && c != EOF){
		c = (char)fgetc(file);
		num_parsed++;
	}
	return num_parsed;
}

uint32_t elfhash(const char* str) {
	assert(str != NULL ); // self-destuct if called with NULL
	uint32_t hashvalue = 0, // value to be returned
		 high; // high nybble of current hashvalue
	while ( *str ) { // continue until *str is '\0'
		hashvalue = (hashvalue << 4) + *str++; // shift high nybble out,
						       // add in next char,
						       // skip to the next char
		if ( high = (hashvalue & 0xF0000000) ) { // if high nybble != 0000
		hashvalue = hashvalue ^ (high >> 24); // fold it back in
		}
		hashvalue = hashvalue & 0x7FFFFFFF; // zero high nybble
	}
	return hashvalue;
}

uint32_t fill_indicies(FILE* file, FIDObject** array, StringHashTable* table, uint32_t array_size){
	uint32_t offset = continue_until(file, '\n');
	char* line_buf = NULL;
	size_t buf_size = 0;
	ssize_t line_len;
	uint32_t numEle = 0;
	line_len = getline(&line_buf, &buf_size, file);
	while (line_len >= 0){
		int id;
		char* name = (char*)malloc(124*sizeof(char));
       		char* state = (char*)malloc(3*sizeof(char));
		sscanf(line_buf, "%i|%[^|]|%*[^|]|%[^|]|", &id, name, state);
		strcat(name, state);
		FIDObject* fid = FIDObject_create(id, offset);
		if (numEle == array_size){
			*array = (FIDObject*)realloc(*array, array_size * sizeof(FIDObject) * 2);
			array_size = array_size * 2;
		}
		(*array)[numEle] = *fid;
		StringHashTable_addEntry(table, name, offset);
		free(name);
		free(state);
		numEle++;
		offset = offset + line_len;
		line_len = getline(&line_buf, &buf_size, file);
		free(fid);
	}
	free(line_buf);
	return numEle;
}

int compare(const void *left, const void *right){
	return (((FIDObject*)left)->fid - ((FIDObject*)right)->fid);
}

void parse_commands(FILE* in, FILE* out, FILE* data, FIDObject* array, StringHashTable* table, uint32_t numEle){
	char* line_buf = NULL;
	size_t buf_size = 0;
	ssize_t line_len = getline(&line_buf, &buf_size, in);
	int cmdNum = 1;
	while (line_len > 0){
		if (line_buf[0] == ';'){
			fprintf(out, line_buf);
		}
		else{
			fprintf(out, "Cmd\t%i:\t%s\n", cmdNum, line_buf);
			cmdNum++;
			char* cmd = (char*)malloc(17*sizeof(char));
			sscanf(line_buf, "%s", cmd);
			if (strcmp(cmd, "exists") == 0){
				char* name = (char*)malloc(124*sizeof(char));
				char* nameCpy = (char*)malloc(124*sizeof(char));
               			char* state = (char*)malloc(3*sizeof(char));
				sscanf(line_buf, "%*s\t%[^\t]%s", name, state);
				strcpy(nameCpy, name);
				strcat(name, state);
				uint32_t* offsets = StringHashTable_getLocationsOf(table, name);
				int numKeys = array_length(offsets);
				fprintf(out, "%i occurrences: %s %s\n", numKeys, nameCpy, state);
				free(nameCpy);
				free(offsets);
				free(name);
				free(state);
			}
			else if (strcmp(cmd, "details_of") == 0){
				char* name = (char*)malloc(124*sizeof(char));
                		char* state = (char*)malloc(3*sizeof(char));
				sscanf(line_buf, "%*s\t%[^\t]%s", name, state);
				strcat(name, state);
				uint32_t* offsets = StringHashTable_getLocationsOf(table, name);
				if (offsets != NULL){
					int numOffsets = array_length(offsets);
					qsort(offsets, numOffsets, sizeof(uint32_t), compare_offsets);
					for (int i = 0; i < numOffsets; i++){
						uint32_t offset = offsets[i];
						int id;
						char* name = (char*)malloc(121*sizeof(char));
						char* class = (char*)malloc(51*sizeof(char));
						char* state = (char*)malloc(3*sizeof(char));
						char* county = (char*)malloc(101*sizeof(char));
						char* latDMS = (char*)malloc(8*sizeof(char));
						char* longDMS = (char*)malloc(9*sizeof(char));
						float latDEC;
						float longDEC;
						fseek(data, offset, SEEK_SET);
						char* line_buf2 = NULL;
						size_t buf_size2 = 0;
						getline(&line_buf2, &buf_size2, data);
						sscanf(line_buf2, "%i|%[^|]|%[^|]|%[^|]|%*[^|]|%[^|]|%*[^|]|%[^|]|%[^|]|%f|%f|", &id, name, class, state, county, latDMS, longDMS, &latDEC, &longDEC);
						fprintf(out, "FID:       %i\n", id);
						fprintf(out, "Name:      %s\n", name);
						fprintf(out, "Type:      %s\n", class);
						fprintf(out, "State:     %s\n", state);
						fprintf(out, "County:    %s\n", county);
						latDMS = separate_coord(latDMS, 0);
						longDMS = separate_coord(longDMS, 1);
						fprintf(out, "Longitude: %s   (%f)\n", longDMS, longDEC);
						fprintf(out, "Latitude:   %s   (%f)\n\n", latDMS, latDEC);
						free(line_buf2);
						free(latDMS);
						free(longDMS);
						free(county);
						free(class);
						free(name);
						free(state);
					}
				}
				else{
					fprintf(out, "No records found for: %s\n", name);
				}
				free(name);
				free(state);
				free(offsets);
			}
			else if (strcmp(cmd, "distance_between") == 0){
				int id1;
				int id2;
				sscanf(line_buf, "%*s\t%i\t%i", &id1, &id2);
				FIDObject* obj1 = FIDObject_create(id1, 0);
				FIDObject* obj2 = FIDObject_create(id2, 0);
				FIDObject* result1 = bsearch(obj1, array, numEle, sizeof(FIDObject), compare);
				FIDObject* result2 = bsearch(obj2, array, numEle, sizeof(FIDObject), compare);
				if (result1 != NULL && result2 != NULL){
					uint32_t offset1 = result1->offset;
					uint32_t offset2 = result2->offset;
					char* name1 = (char*)malloc(121*sizeof(char));
                                	char* state1 = (char*)malloc(3*sizeof(char));
                                	char* latDMS1 = (char*)malloc(8*sizeof(char));
                                	char* longDMS1 = (char*)malloc(9*sizeof(char));
                                	float latDEC1;
                                	float longDEC1;
					char* name2 = (char*)malloc(121*sizeof(char));
                                	char* state2 = (char*)malloc(3*sizeof(char));
                                	char* latDMS2 = (char*)malloc(8*sizeof(char));
                                	char* longDMS2 = (char*)malloc(9*sizeof(char));
                                	float latDEC2;
                                	float longDEC2;
					fseek(data, offset1, SEEK_SET);
					char* line_buf2 = NULL;
                                	size_t buf_size2 = 0;
                                	getline(&line_buf2, &buf_size2, data);
					sscanf(line_buf2, "%*i|%[^|]|%*[^|]|%[^|]|%*[^|]|%*[^|]|%*[^|]|%[^|]|%[^|]|%f|%f|", name1, state1, latDMS1, longDMS1, &latDEC1, &longDEC1);
					fseek(data, offset2, SEEK_SET);
					getline(&line_buf2, &buf_size2, data);
					sscanf(line_buf2, "%*i|%[^|]|%*[^|]|%[^|]|%*[^|]|%*[^|]|%*[^|]|%[^|]|%[^|]|%f|%f|", name2, state2, latDMS2, longDMS2, &latDEC2, &longDEC2);
					latDMS1 = separate_coord(latDMS1, 0);
					longDMS1 = separate_coord(longDMS1, 1);
					latDMS2 = separate_coord(latDMS2, 0);
                                	longDMS2 = separate_coord(longDMS2, 1);
					latDEC1 = latDEC1 * PI / 180;
					longDEC1 = longDEC1 * PI / 180;
					latDEC2 = latDEC2 * PI / 180;
					longDEC2 = longDEC2 * PI / 180;
					float cent_ang = acos(sin(latDEC1)*sin(latDEC2) + cos(latDEC1)*cos(latDEC2)*cos(fabs(longDEC1 - longDEC2)));
					float length = round(RADIUS * cent_ang * 10) / 10;
					fprintf(out, "First:     ( %s,  %s )  %s, %s\n", longDMS1, latDMS1, name1, state1);
					fprintf(out, "Second:    ( %s,  %s )  %s, %s\n", longDMS2, latDMS2, name2, state2);
					fprintf(out, "Distance:  %.1fkm\n", length);
					free(line_buf2);
					free(name1);
					free(state1);
					free(latDMS1);
					free(longDMS1);
					free(name2);
					free(state2);
					free(latDMS2);
					free(longDMS2);
					free(obj1);
					free(obj2);
				}
				else{
					fprintf(out, "No record was found for one or both of: %i, %i\n", id1, id2);
				}
			}
			free(cmd);
			fprintf(out, "----------------------------------------------------------------------\n");
		}
		line_len = getline(&line_buf, &buf_size, in);
	}
	free(line_buf);
}

int array_length(uint32_t* array){
	int idx = 0;
	while (array[idx] != 0){
		idx++;
	}
	return idx;
}

int compare_offsets(const void* left, const void* right){
	return (*((uint32_t*)left) - *((uint32_t*)right));
}

char* separate_coord(char* coord, int lalo){
	if (coord[0] == '0'){
		memmove(coord, coord+1, strlen(coord));
	}
	int d; 
	int m;
	int s;
	char direct;
	int num;
	sscanf(coord, "%i%c", &num, &direct);
	d = floor(num / 10000);
        m = floor((num % 10000) / 100);
        s = (num % 100);
	char* nsew;
	if (direct == 'N'){
		nsew = "North";
	}
	else if (direct == 'S'){
		nsew = "South";
	}
	else if (direct == 'E'){
		nsew = "East";
	}
	else if (direct == 'W'){
		nsew = "West";
	}
	char* result = (char*)malloc(19*sizeof(char));
	if (lalo == 0){
		sprintf(result, "%id %im %is %s", d, m, s, nsew);
	}
	else{
		sprintf(result, "%03id %im %is %s", d, m, s, nsew);
	}
	free(coord);
	return result;
}
