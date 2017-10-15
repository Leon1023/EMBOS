#include "storage.h"

#define MAX_SUPER_BLOCK		(8)
#define ROMFS			0

struct super_block;
struct inode{
	char *name;		//file name
	unsigned int flags;	//for file control
	size_t dsize;		//real data size, except for file head
	unsigned int daddr;	//address of file,contain data and file head  
	struct super_block *super;//the control functions for the file system
};


struct super_block{
	//namei:get a file's inode by file name
	struct inode*(*namei)(struct super_block *super,char *p);
	//get_daddr:return address of real data
	unsigned int (*get_addr)(struct inode *);
	//the storage which the file system on
	struct storage_device *device;
	//the name of the file system
	char *name;
}
//the all file system type which the OS have
extern struct super_block *fs_type[];

