#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include "fs_emulator.h"

int main(int argc, char* argv[]) { 
	FILE *fp;
	int check;  // check for directory operation
	InodeList inodes;  // declare an instance of InodeList
	inodes.inodeCount = 0;  // initialize count to 0
	inodes.currentInode = 0;  // initialize current inode to 0	
	char cmd[2048];	

	if (argc == 2) {
		check = chdir(argv[1]);
		if (check == -1) {
			perror("Directory Operation Unsuccessful");
			return 1;
		}
	}
	else {
		printf("Invalid number of arguments\n");
		return 1;
	}
	
	loadInodeList("inodes_list", &inodes);  // load inodes
	if (inodes.inodeList[0].type != 'd') {
		fprintf(stderr, "ERROR: inode 0 not a directory (not in use)\n");
		return 1;
	}
	char *rootname = "/";
	strcpy(inodes.inodeList[0].name, rootname);  // root directory's name is '\'
	inodes.inodeList[0].parentInode = -1; // root directory has no parent

	fp = stdin;  // read from standard input
	while (fgets(cmd, 2048, fp) != NULL) {
		remove_newline(cmd);  // remove newline, replace with null char
		remove_trailing_whitespace(cmd);
		if (strcmp(cmd, "exit") == 0) {
			break;
		}
		else if (strncmp(cmd, "cd", 2) == 0 && (cmd[2] == '\0' || cmd[2] == ' ')) {
			if (is_valid(cmd) == 0) {
				printf("cd: too many arg\n");
			}
			else if (strlen(cmd) > 3) {
				changeDirectory(cmd, &inodes);
			}
			else {
				printf("cd: provide a directory\n");
			}	
		}
		else if (strcmp(cmd, "ls") == 0) {
			listContents(&inodes);
		}
		else if (strncmp(cmd, "mkdir", 5) == 0  && (cmd[5] == '\0' || cmd[5] == ' ')) {		
			cmd[38] = '\0';  // truncates name arg to max length of 32
			if (is_valid(cmd) == 0) {
				printf("mkdir: too many arg\n");
			}
			else if (strlen(cmd) > 6) {
				createDirectory(cmd, &inodes);
			} 
			else {
				printf("mkdir: missing operand\n");
			}
		}
		else if (strncmp(cmd, "touch", 5) == 0 && (cmd[5] == '\0' || cmd[5] == ' ')) {
			cmd[38] = '\0';  // truncates name arg to max length of 32
			if (is_valid(cmd) == 0) {
				printf("touch: too many arg\n");
			}
			else if (strlen(cmd) > 6) {
				createFile(cmd, &inodes);
			}
			else {
				printf("touch: missing operand\n");
			}
		}
		else {
			printf("fs-emulator: command '%s' not found\n", cmd);
		}
	}
	saveInodeList("inodes_list", &inodes);	
	fclose(fp);
	return 0;
}

void loadInodeList(const char *path, InodeList *inodes) {
	FILE *fp;
	size_t bytes = 0;
	size_t SIZE = 5;  // read 5 bytes at a time
	char buffer[2048];
	Inode tempNode;  // tempnode for insertion

	fp = fopen(path, "r");  // open inodeList which contains given number of inodes
	if (fp == NULL) {
		perror("File Operation Unsuccessful");
	}
	while ((bytes = fread(buffer, 1, SIZE*sizeof(char), fp)) == SIZE*sizeof(char)) {
		if (0 <= (int)buffer[0] && (int)buffer[0] < 1024) {
			tempNode.inode = buffer[0];
			tempNode.type = buffer[4];
			inodes->inodeList[(int)buffer[0]] = tempNode;
			inodes->inodeCount++;  // update the inodeCount
		}
		else {
			printf("invalid inode with value '%d' detected", (int)buffer[0]);
		}
	}
	
	fclose(fp);

	// for each node in Inode list
	// if a directory assign directory name, assign file name, assign parent
	for (int i = 0; i < inodes->inodeCount; i++) {
		if (inodes->inodeList[i].type == 'd') {
			sprintf(buffer, "%d", i);
			fp = fopen(buffer, "r");	
			//printf("in directory");
			if (fp == NULL) {
				perror("File Operation Unsuccessful");	
			}
			
			bytes = fread(buffer, 1, 36*2*sizeof(char), fp);
			if (bytes != 36*2*sizeof(char)) {
				fprintf(stderr, "Invalid Directory\n");
			}
			inodes->inodeList[i].parentInode = buffer[36];
			int parent = buffer[0];	
			while ((bytes = fread(buffer, 1, 36*sizeof(char), fp)) == 36*sizeof(char)) {
				strcpy(inodes->inodeList[(int)buffer[0]].name, buffer + 4*sizeof(char));
				inodes->inodeList[(int)buffer[0]].parentInode = parent;
				//printf("%s\n", inodes->inodeList[(int)buffer[0]].name);
			}	
			fclose(fp);
		}
	}	
}

void saveInodeList(const char *path, InodeList *inodes) {
	FILE *fp;
	char buffer[5] = {'\0'};
	fp = fopen(path, "w");
	for (int i = 0; i < inodes->inodeCount; i++) {
		buffer[0] = i;
		buffer[4] = inodes->inodeList[i].type;
		if (fwrite(buffer, sizeof(char), 5, fp) != 5) {
			perror("write failed");
		}
	}  // writes the content of the inodeList to the file path [inodes_list]
	fclose(fp);  // close when done	
}

void changeDirectory(const char *name, InodeList *inodes) {
	name = name + 3*sizeof(char);
	for (int i = 0; i < inodes->inodeCount; i++) {
		if (inodes->inodeList[i].type == 'd' && strcmp(name, inodes->inodeList[i].name) == 0) {
			inodes->currentInode = i;  // set currentInode value to the index of the list with that name
			return;
		}  // iterate through the inodeList to search whether a directory with the name already exists
	}
	printf("cd: directory '%s' does not exist\n", name);  // otherwise print an error message; directory not found
}

void listContents(InodeList *inodes) {	
	for (int i = 0; i < inodes->inodeCount; i++) {  // iterate through inodeList
		if (inodes->inodeList[i].parentInode == inodes->currentInode){  // if inode's parent is current inode
			printf("%i: %s [%c]\t\t", i, inodes->inodeList[i].name, inodes->inodeList[i].type);  // print members of current inode
		}
	}
	printf("\n");
} 

void createDirectory(const char *name, InodeList *inodes){
	name = name + 6*sizeof(char);
	FILE *fp;
	char fileID[10];
	char writeBuffer[36] = {'\0'};
	
	// -- CHECK IF INODE LIST IS FULL or IF DIRECTORY NAME ALREADY USED --
        if (inodes->inodeCount == 1024) {
                return;
        }  // check to see if the inodeCount is 1024; if yes, exit
        for (int i = 0; i < inodes->inodeCount; i++) {
                if (strcmp(name, inodes->inodeList[i].name) == 0 && inodes->inodeList[i].type == 'd') {
                        return;
                }
        }  // iterate through inodeList to see if directory name exists; if yes, exit 

	// -- CREATE AND ADD DIRECTORY INODE TO INODE LIST --
	Inode tempNode;
        tempNode.inode = inodes->inodeCount;  // set inode ID (next number is inodeCount)
        tempNode.parentInode = inodes-> currentInode;  // set parent inode ID to currentInode (current working directory)
        tempNode.type = 'd';  // set inode type to directory
        strcpy(tempNode.name, name);  // save to inode its directory name
        inodes->inodeList[inodes->inodeCount] = tempNode; // append inode to inode list
        inodes->inodeCount++;  // increment inode list counter
	// create a new inode with new inodeCount, set its parentInode and type, and also its name

	// -- WRITE INTO NEW DIRECTORY --
	sprintf(fileID, "%d", (int)(inodes->inodeCount - 1));  // convert int of new iNodeID to string
        fp = fopen(fileID, "w");  // open a new directory file in write mode 
        if (fp == NULL) {
                perror("File Operation Unsuccessful");
                return;
        }  // file checking
	// create a file with its name as the inode number
	writeBuffer[0] = (char)((inodes->inodeCount) - 1);  // inode number
	writeBuffer[4] = '.';  // single dot 
	if (fwrite(writeBuffer, 1, sizeof(writeBuffer), fp) != sizeof(writeBuffer)) {
		perror("Write Failed");
		return;
	}  // write first 36 bytes to directory file (its . inode values)
	writeBuffer[0] = (char)(inodes->inodeList[inodes->inodeCount - 1].parentInode);  // parent inode number
	writeBuffer[5] = '.';  // double dot
        if (fwrite(writeBuffer, 1, sizeof(writeBuffer), fp) != sizeof(writeBuffer)) {
                perror("Write Failed");
                return;
        }  // write next 36 bytes to directory file (its .. inode values)
        
	fclose(fp);  // close new file

	// -- WRITE INTO WORKING DIRECTORY THE NEW DIRECTORY --
	sprintf(fileID, "%d", (int)inodes->currentInode);  // convert inodeID of current directory to string
	fp = fopen(fileID, "a");  // open working directory file
	if (fp == NULL) {
		perror("File Operation Unsuccessful");
		return;
	}
        
        writeBuffer[0] = (char)((inodes->inodeCount) - 1);  // add inode ID of new directory to write buffer
        int j = 4;  // name begins at 4 byte offset
        while (*name != '\0' && j < 36) {
                writeBuffer[j] = *name;  // write out its name
                j++;
                name++;
        }  // add inode Name of new directory to write buffer

        if (fwrite(writeBuffer, 1, sizeof(writeBuffer), fp) != sizeof(writeBuffer)) {
                perror("write failed");
        }  // write new inode's ID and name to current working directory file using write buffer

        fclose(fp);  // close directory file	
}

void createFile(const char *name, InodeList *inodes){
	name = name + 6*sizeof(char);
	FILE *fp;
	const char *temp = name;
        char fileID[10];
        char writeBuffer[36] = {'\0'};
	char newline = '\n';

	// -- CHECK IF INODE LIST IS FULL or IF FILENAME ALREADY USED --
	if (inodes->inodeCount == 1024) {
		return;
	}  // check if the inodeCount is 1024; if yes, exit
	for (int i = 1; i < inodes->inodeCount; i++) {
		if (strcmp(name, inodes->inodeList[i].name) == 0 && inodes->inodeList[i].type == 'f') {
			return;
		}
	}  // iterate through inodeList to see if filename exists; if yes, exit

	// -- CREATE AND ADD FILE INODE TO INODE LIST --
	Inode tempNode;
	tempNode.inode = inodes->inodeCount;  // set inode ID (next number is inodeCount)
	tempNode.parentInode = inodes-> currentInode;  // set parent inode ID to currentInode (current working directory)
	tempNode.type = 'f';  // set inode type to file
	strcpy(tempNode.name, name);  // save to inode its filename
	inodes->inodeList[inodes->inodeCount] = tempNode; // append inode to inode list
	inodes->inodeCount++;  // increment inode list counter

	// -- WRITE INTO NEW FILE --
        sprintf(fileID, "%d", (int)inodes->inodeCount - 1);  // convert int of new iNodeID to string
        fp = fopen(fileID, "w");  // open a new file with name as next iNodeID
        if (fp == NULL) {
                perror("File Operation Unsuccessful");
                return;
        }  // file checking

        while (*temp != '\0') {
                if (fwrite(temp, 1, sizeof(char), fp) != 1) {
                        perror("Write Failed");
                        return;
                }
                temp++;
        }  // writes to filename to file

        if (fwrite(&newline, 1, sizeof(char), fp) != 1) {
                perror("write failed");
        } // add newline to end of new file

        fclose(fp);  // close new file

	// -- WRITE INTO WORKING DIRECTORY THE NEW FILE --
	sprintf(fileID, "%d", (int)inodes->currentInode);  // convert inodeID of current directory to string
	fp = fopen(fileID, "a");  // open working directory file
	if (fp == NULL) {
		perror("File Operation Unsuccessful");
		return;
	}

	writeBuffer[0] = (char)((inodes->inodeCount) - 1);  // add inode ID of new file to write buffer
	int j = 4;  // name begins at 4 byte offset
	while (*name != '\0' && j < 36) {
		writeBuffer[j] = *name;  // write out its name
		j++;
		name++;
	}  // add inode Name of new file to write buffer

	if (fwrite(writeBuffer, 1, sizeof(writeBuffer), fp) != sizeof(writeBuffer)) {
		perror("write failed");
	}  // write new inode's ID and name to current working directory file using write buffer

	fclose(fp);  // close directory file
}

void remove_newline(char *buffer) {
	while (*buffer != '\n') {
		buffer++;
	}
	*buffer = '\0';
}  // removes and replaces newline with null character

void remove_trailing_whitespace(char *buffer) {
	while (*buffer != '\0') {
		buffer++;
	}
	buffer--;  // start a end char
	while (*buffer == ' ') {
		*buffer = '\0';
		buffer--;
	}  // remove any whitespace on the end
}

int is_valid(char *buffer) {
	int whitespace_cnt = 0;
	while (*buffer != '\0') {
		if (*buffer == ' ') {
			whitespace_cnt++;
		}	
		if (whitespace_cnt == 2) {  // more than one arg provided
			return 0;  // return false
		}
		buffer++;
	}
	return 1;  // return true
}
