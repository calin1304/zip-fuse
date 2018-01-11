#ifndef FS_TREE_H
#define FS_TREE_H

#include <stdlib.h>
#include <zip.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <collectc/array.h>

#define ZIP_FILE_FLAG_TYPE_FILE 0x1
#define ZIP_FILE_FLAG_TYPE_DIR	0x2

struct fs_node {
	char *name;
	int type;
	zip_stat_t fstat;
	struct stat st;
	Array *desc;
};

struct fs_tree {
	struct fs_node *root;
};

typedef struct fs_node fs_node_t;
typedef struct fs_tree fs_tree_t;

void fs_tree_init(fs_tree_t *r);
void fs_tree_free(fs_tree_t *r);
fs_tree_t build_fs_tree_from_zip(zip_t *z);

struct fs_node* fs_tree_get_node_from_path(const struct fs_tree* r, const char *path);

#endif
