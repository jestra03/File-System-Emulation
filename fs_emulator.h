#ifndef EMULATOR_H
#define EMULATOR_H

typedef struct Inode {
	uint32_t inode;  // inode name
	uint32_t parentInode;  // inode's parent
	char type;  // two types, directory or file
	char name[32];  // name of directory or file
} Inode;

typedef struct InodeList {
	Inode inodeList[1024];  // array of Inodes
	size_t inodeCount;  // counter of inodes in list, initialize at zero
	uint32_t currentInode;  // tracks current node, initalize at zero
} InodeList;

void loadInodeList(const char *path, InodeList *inodes);
void saveInodeList(const char *path, InodeList *inodes);
void changeDirectory(const char *name, InodeList *inodes);
void listContents();
void createDirectory(const char *name, InodeList *inodes);
void createFile(const char *name, InodeList *inodes);

void remove_newline(char *buffer);
void remove_trailing_whitespace(char *buffer);
int is_valid(char *buffer);

#endif
