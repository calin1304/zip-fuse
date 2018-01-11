#include "fs_tree.h"

#include <string.h>

#include "log.h"

void fs_tree_init(fs_tree_t *r)
{
	r->root = fs_node_create("/", ZIP_FILE_FLAG_TYPE_DIR);
}

void fs_tree_free(fs_tree_t *r)
{
	fs_node_free(r->root);
	free(r->root);
}

fs_node_t* fs_tree_add_path(fs_tree_t *r, const char *path)
{
	char *p = strdup(path);
	char *q = strtok(p, "/");
	fs_node_t *currNode = r->root;
	while (q) {
		fs_node_t *childNode = fs_node_find_desc(currNode, q);
		if (childNode == NULL) {
			childNode = fs_node_create(strdup(q), ZIP_FILE_FLAG_TYPE_DIR);
			array_add(currNode->desc, childNode);
		}
		currNode = childNode;
		q = strtok(NULL, "/");
	}
	free(p);
	if (path[strlen(path) - 1] != '/') {
		fs_node_set_type(currNode, ZIP_FILE_FLAG_TYPE_FILE);
	}
	return currNode;
}

void fs_node_set_stat_from_zip_stat(fs_node_t *r, const zip_stat_t *zstat)
{
	r->st.st_size = zstat->size;
	r->st.st_mtime = zstat->mtime;
}

fs_tree_t build_fs_tree_from_zip(zip_t *z)
{
	fs_tree_t ret;
	fs_tree_init(&ret);
	int num_entries = zip_get_num_entries(z, 0);
	// TODO: i should be zip_uint64_t
	// num_entries should not be zip_uint64_t because
	// zip_get_num_entries return -1 if archive is NULL;
	for (int i = 0; i < num_entries; ++i) {
		const char *name = zip_get_name(z, i, 0);
		log_debug("Adding %s to filesystem tree", name);
		fs_node_t *p = fs_tree_add_path(&ret, name);
		zip_stat_index(z, i, 0, &(p->fstat));
		fs_node_set_stat_from_zip_stat(p, &(p->fstat));
	}
	return ret;
}

struct fs_node* fs_tree_get_node_from_path(const struct fs_tree* r, const char *path)
{
	fs_node_t *currNode = r->root;
	const char *p = path + 1;
	const char *q = strchr(p, '/');;
	while (q) {
		currNode = fs_node_find_desc_n(currNode, p, q - p);
		p = q + 1;
		q = strchr(p, '/');;
	}
	q = strchr(p, '\0');
	currNode = fs_node_find_desc_n(currNode, p, q - p);
	return currNode;
}
