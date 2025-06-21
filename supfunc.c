#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

char *uint32_to_str(uint32_t i)
{
   int length = snprintf(NULL, 0, "%lu", (unsigned long)i);   // pretend to print to a string to determine length
   char *str = malloc(length + 1);   // allocate space for the actual string

   snprintf(str, length + 1, "%lu", (unsigned long)i);        // print to string

   return str;
}

FILE *isdir(uint32_t i, char c) {
	char *inode = uint32_to_str(i);
	FILE *fp = fopen(inode, "rb");

	if (fp == NULL) {
                printf("Inode %u inaccessible, terminating.\n", i);
                return fp;
        } else if (c != 'd') {
                printf("Inode %u is a not a directory.\n", i);
                fclose(fp);
                return NULL;
        } else {
		fclose(fp);
		fp = fopen(inode, "ab+");
		free(inode);
		return fp;
	}
}

int loadir(FILE *fp, char dir[][32], uint32_t *node) {
	int i;
        for (i = 0; i < 1024; i++) {
                fread(&node[i], sizeof(uint32_t), 1, fp);
                fread(dir[i], 32, 1, fp);
                if (feof(fp)) {
                        return i;
                }
        }
	return -1;
}

void rmnewline(char *in) {
	if (in) {
	if (in[strlen(in) - 1] == '\n') {
                       in[strlen(in) - 1] = 0;
        }
	}
}
