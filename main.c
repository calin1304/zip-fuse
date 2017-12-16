#define FUSE_USE_VERSION 31

#include <zip.h>
#include <fuse.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>

#include "fs_tree.h"

const char input_zip[] = "test.zip";

fs_tree g_fs_tree;

static int cfs_getattr(const char *path, struct stat *st,
		struct fuse_file_info *fi)
{
	(void) fi;

	if (strcmp(path, "/") == 0) {
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2;
	} else if (strcmp(path+1, "hello.txt") == 0) {
		st->st_mode = S_IFREG | 0444;
		st->st_nlink = 1;
		st->st_size = 0;
	} else {
		return -ENOENT;
	}

	return 0;
}

static int cfs_readdir(const char *path, void *buffer,
		fuse_fill_dir_t filler, off_t offset,
		struct fuse_file_info *fi, enum fuse_readdir_flags flags)
{
	(void) fi;
	(void) offset;
	(void) flags;

	if (strcmp(path, "/") != 0) {
		return -ENOENT;
	}

	filler(buffer, ".", NULL, 0, 0);
	filler(buffer, "..", NULL, 0, 0);
	unsigned int n;
	char **entries = fs_tree_readdir(path, &n);
	for (unsigned int i = 0; i < n; ++i) {
		filler(buffer, entries[i], NULL, 0, 0);
	}

	return 0;
}

static struct fuse_operations ffs_oper = {
	.getattr 	= cfs_getattr,
	.readdir	= cfs_readdir
};

void _fs_tree_debug_print(fs_node_t *r, int depth)
{
	for (int i = 0; i < depth; ++i) {
		printf("\t");
	}
	printf("-> %s desc: %d\n", r->name, r->num_desc);
	for (int i = 0; i < r->num_desc; ++i) {
		_fs_tree_debug_print(r->desc[i], depth+1);
	}
}

void fs_tree_debug_print(fs_tree_t *r)
{
	_fs_tree_debug_print(r->root, 0);	
}

int main(int argc, char** argv)
{
	zip_t *z = zip_open(input_zip, ZIP_RDONLY, NULL);
	g_fs_tree = build_fs_tree_from_zip(z);
	zip_close(z);
	return EXIT_SUCCESS;
	//return fuse_main(argc, argv, &ffs_oper, NULL);
}

