#include "fs_tree.h"

#include <string.h>

#include "log.h"

struct fs_node* fs_node_find_desc_n(struct fs_node *r, const char *name, size_t sz);
fs_node_t* fs_node_find_desc(fs_node_t *r, const char *name);

void fs_node_set_type(fs_node_t *r, int type)
{
	if (type == ZIP_FILE_FLAG_TYPE_FILE) {
		r->st.st_mode = S_IFREG | 0444;
	} else {
		r->st.st_mode = S_IFDIR | 0755;
	}
	r->type = type;
}

fs_node_t* fs_node_create(char *name, int type)
{
	fs_node_t *ret = malloc(sizeof(fs_node_t));
	memset(ret, 0, sizeof(fs_node_t));
	ret->name = name;
	zip_stat_init(&ret->fstat);
	memset(&(ret->st), 0, sizeof(struct stat));
	fs_node_set_type(ret, type);
	ret->desc = malloc(sizeof(fs_node_t) * 4);
	ret->num_desc = 0;
	ret->desc_capacity = 4;
	return ret;
}

void fs_node_add_desc(fs_node_t *r, fs_node_t *desc)
{
	if (r->num_desc  == r->desc_capacity) {
		size_t new_size = r->desc_capacity * 2 * sizeof(fs_node_t*);
		r->desc = realloc(r->desc, new_size);
	}
	r->desc[r->num_desc++] = desc;
}

void fs_tree_init(fs_tree_t *r)
{
	r->root = fs_node_create("/", ZIP_FILE_FLAG_TYPE_DIR);
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
			fs_node_add_desc(currNode, childNode);
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
	}
	return ret;
}

struct fs_node* fs_node_find_desc_n(struct fs_node *r, const char *name, size_t sz)
{
	if (sz == 0) {
		return r;
	}
	for (unsigned int i = 0; i < r->num_desc; ++i) {
		if (strncmp(name, r->desc[i]->name, sz) == 0) {
			return r->desc[i];
		}
	}
	return NULL;
}

fs_node_t* fs_node_find_desc(fs_node_t *r, const char *name)
{
	return fs_node_find_desc_n(r, name, strlen(name));
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
