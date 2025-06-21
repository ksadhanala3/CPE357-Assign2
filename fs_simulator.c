#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include"supfunc.c"

int main(int argc, char *argv[]) {
	char inodes[1024];
	uint32_t i, temp;
	char type;


	//Check if directory exists
	if (chdir(argv[1])) {
		printf("%s is not a directory.\n", argv[1]);
		return 1;
	}
	
	//Check if list exists
	FILE *list = fopen("inodes_list", "rb");

	if (list == NULL) {
		printf("Inodes_list inaccessible, terminating.\n");
		return 0;
	}
	fclose(list);

	//Reopen list in append read mode and store type in inode index
	list = fopen("inodes_list", "ab+");
	for (i = 0; i < 1024 && !feof(list); i++) { 
		fread(&temp, sizeof(uint32_t), 1, list);
		fread(&type, 1, 1, list);
		if (type == 'd' || type == 'f') {
			if (temp == i) {
				inodes[i] = type;
			} else {
				break;
			}
		} else {
			printf("Indicator invalid.\n");
		}
	}
	
	
	/*
	 *print out list of inodes
	 *for (int j = 0; j < i; j++) {
	 *       printf("%d %c\n", j, inodes[j]);
	 *}
	 *printf("Inode index i %u %s\n", i, uint32_to_str(i));
	 */	

	//check if 0 is dir and openable otherwise exit	
	FILE *dflist = isdir(0, inodes[0]);

	if (!dflist) {
		return 0;
	}
	
	//Takes file pointer from isdir and reopens in append read binary
	char dir[1024][32], nam[32] = {0}, cmd[38], *in, *name;
	uint32_t node[1024];
	int index = loadir(dflist, dir, node);
	int flag;
	FILE *prev = dflist; //store previous dir

	do {
		printf("> ");
		fgets(cmd, 38, stdin);
	        in = strtok(cmd, " ");
		name = strtok(NULL, " ");
		rmnewline(in);
		rmnewline(name);

		if (name) {			//hacky code to stop valgrind initialize warning
			strcpy(nam, name);
		}
			flag = 1;

		//Simply print out inode and name
		if (!strcmp(cmd, "ls")) {
			for (int j = 0; j < index; j++) {
				printf("%u %s\n", node[j], dir[j]);
			}
		}

		// cd check name was entered, check if correct, then check if dir and load
		// need to implement name protections and errors
		else if (!strcmp(cmd, "cd")) {
			if (!name) {
				printf("Directory name not provided\n");
				continue;
			}
			for (int j = 0; j < index; j++) {
				if (!strcmp(dir[j], name)) {
					dflist = isdir(node[j], inodes[node[j]]);
					//printf("%c\n", inodes[node[j]]);
					if (!dflist) {
						dflist = prev;
						flag = 0;
						break;
					}
					index = loadir(dflist, dir, node);
					flag = 0;
					break;
				}
			}
			if (flag) {
				printf("Directory name invalid.\n");
			}
		}

		//mkdir
		else if (!strcmp(cmd, "mkdir")) {
			if (!name) {
				printf("Directory name not provided.\n");
				continue;
			}
			for (int k = 0; k < index; k++) {
				if (!strcmp(dir[k], name)) {
						printf("Directory already exists.\n");
						flag = 0;
						break;
				}
			}
			if (flag) {
				//create and write to inode file for new dir
				char *intstr = uint32_to_str(i);

				prev = fopen(intstr, "wb");
				fwrite(&i, sizeof(uint32_t), 1, prev);
				fwrite(".", 32, 1, prev);
				fwrite(&node[0], sizeof(uint32_t), 1, prev);
				fwrite("..", 32, 1, prev);
				fclose(prev);
				free(intstr);
				prev = dflist;

				//write new dir in cwd inode file and to node array and dir array
				fwrite(&i, sizeof(uint32_t), 1, dflist);
				fwrite(nam, 32, 1, dflist);
				node[index] = i;
				strcpy(dir[index], name);
				index++;

				//update inodes_list and internal inodes array
				fwrite(&i, sizeof(uint32_t), 1, list);
				type = 'd';
				fwrite(&type, 1, 1, list);
				inodes[i] = type;
				i++;
			}	
		}

		//touch
		else if (!strcmp(cmd, "touch")) {
			if (!name) {
				printf("File name not provided.\n");
				continue;
			}
			for (int k = 0; k < index; k++) {
				if (!strcmp(dir[k], name)) {
					printf("File already exists.\n");
					flag = 0;
					break;
				}
			}
			if (flag) {
				//create and write to inode file for new file
				char *intstr = uint32_to_str(i);

				prev = fopen(intstr, "ab");
				fwrite(name, strlen(name), 1, prev);
				type = '\n';
				fwrite(&type, 1, 1, prev);
				fclose(prev);
				free(intstr);
				prev = dflist;

				//write new file to cwd inode file and update cwd node and dir arrays
				fwrite(&i, sizeof(uint32_t), 1, dflist);
				fwrite(nam, 32, 1, dflist);
				//fwrite('\0', 1, 32 - strlen(name), dflist);
				node[index] = i;
				strcpy(dir[index], name);
				index++;

				//update inodes_list and inodes array
				fwrite(&i, sizeof(uint32_t), 1, list);
				type = 'f';
				fwrite(&type, 1, 1, list);
				inodes[i] = type;
				i++;
			}
		}


		fflush(dflist);
		fflush(list);
		//printf("%s,%s,%s,\n", cmd, in, name);
	} while (strcmp(cmd, "exit"));

	fclose(dflist);	
	fclose(list);
	return 0;
}
