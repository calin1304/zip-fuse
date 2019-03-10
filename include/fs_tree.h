#ifndef FS_TREE_H
#define FS_TREE_H

#include <stdlib.h>
#include <zip.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <collectc/array.h>

#include "fs_node.h"
#include "defines.h"

struct fs_tree {
	struct fs_node *root;
};

typedef struct fs_tree fs_tree_t;

void fs_tree_free(fs_tree_t *r);
fs_tree_t build_fs_tree_from_zip(zip_t *z);

struct fs_node* fs_tree_get_node_from_path(const struct fs_tree* r, const char *path);

#endif
