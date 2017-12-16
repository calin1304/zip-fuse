#ifndef FS_TREE_H
#define FS_TREE_H

#define FS_DIR 	1
#define FS_FILE 2

#include <stdlib.h>
#include <zip.h>

struct fs_node {
	char *name;
	int type;
	struct fs_node **desc;	
	size_t num_desc;
	size_t desc_capacity;
};

struct fs_tree {
	struct fs_node *root;
};

typedef struct fs_node fs_node_t;
typedef struct fs_tree fs_tree_t;

void fs_tree_init(fs_tree_t *r);
fs_tree_t build_fs_tree_from_zip(zip_t *z);

char **fs_tree_readdir(const char *path, int *entries_count);

#endif
