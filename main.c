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

fs_tree_t g_fs_tree;

static int cfs_getattr(const char *path, struct stat *st,
		struct fuse_file_info *fi)
{
	(void) fi;

	fs_node_t *p = fs_tree_get_node_from_path(&g_fs_tree, path);
	if (p == NULL) {
		return -ENOENT;
	}
	*st = p->st;
	return 0;
}


static int cfs_readdir(const char *path, void *buffer,
		fuse_fill_dir_t filler, off_t offset,
		struct fuse_file_info *fi, enum fuse_readdir_flags flags)
{
	(void) fi;
	(void) offset;
	(void) flags;

	filler(buffer, ".", NULL, 0, 0);
	filler(buffer, "..", NULL, 0, 0);
	struct fs_node *p = fs_tree_get_node_from_path(&g_fs_tree, path);
	if (p == NULL) {
		return -ENOENT;
	}
	for (size_t i = 0; i < p->num_desc; ++i) {
		filler(buffer, p->desc[i]->name, NULL, 0, 0);
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
	printf("-> %s\n", r->name, r->num_desc);
	for (unsigned int i = 0; i < r->num_desc; ++i) {
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
	// return EXIT_SUCCESS;
	return fuse_main(argc, argv, &ffs_oper, NULL);
}

