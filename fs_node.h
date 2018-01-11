#ifndef FS_NODE_H
#define FS_NODE_H

#include <fcntl.h>
#include <sys/stat.h>

#include <zip.h>

#include <collectc/array.h>

#include "defines.h"

struct fs_node {
	char *name;
	int type;
	zip_stat_t fstat;
	struct stat st;
	Array *desc;
};

typedef struct fs_node fs_node_t;

fs_node_t* fs_node_create(char *name, int type);
void fs_node_free(fs_node_t *r);
void fs_node_set_type(fs_node_t *r, int type);
fs_node_t* fs_node_find_desc(fs_node_t *r, const char *name);
struct fs_node* fs_node_find_desc_n(struct fs_node *r, const char *name, size_t sz);

#endif