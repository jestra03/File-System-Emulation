FILE EMULATOR
This program emulates a file system using a linear data structure 
utilizing an inode struct which stores information (metadata) 
pertaining to an individual inode "file" (of type file or directory). 

Directory file: 
- contains its inodeID and parentInodeID (first 72 bytes)
- map names to their corresponding inodes (36 bytes per entry)

File file:
- contains the name of the file

Inodes_list file:
- contains information of individual inodes; inodeID and inode type

-- Data Structure --
Inode: Contains information about an inode file
1. inode name (as unsigned int)
2. parent inode name (as unsigned int)
3. file type (as char)
4. name (as string; char array)

InodeList: Contains array of inode objects; information for navigation
1. inodeList (arr of Inodes)
2. inocdeCount (as unsigned int)
3. currentInode (as unsigned int)

-- Commands and Functionalities --
change directory || cd <name> || cd .. || cd /
1. opens given directory if exist; report error otherwise
2. change working directory to parent directory
3. sets current working directory to root directory
*directory names exceeding length of 32 characters are invalid; reports error

list contents || ls
1. lists contents in current working directory (names and inode type)

make directory || mkdir <name> 
1. creates new directory given name; report if it already exists otherwise
	- if name exceeds limit its name is truncated to 32 characters
	- will not add file if inode limit has been reached (1024)
	- errors || multiple args || no arg provided

create file || touch <name>
1. creates new file with given name; report if it already exits otherwise
	- if name exceeds limit its name is truncated to 32 characters
	- will not add directory if inode limit has been reached (1024)
	- errors || multiple args || no arg provided
exit
1. updates state of the inode list, then terminates program

other notes:
- trailing whitespaces are removed (ignored like in standard unix commands)

