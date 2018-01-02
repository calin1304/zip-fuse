#include "fs_tree.h"

#include <string.h>

#include "log.h"

fs_node_t* fs_node_create(char *name, int type)
{
	fs_node_t *ret = malloc(sizeof(fs_node_t));
	memset(ret, 0, sizeof(fs_node_t));
	ret->name = name;
	zip_stat_init(&ret->fstat);
	memset(&(ret->st), 0, sizeof(struct stat));
	if (type == ZIP_FILE_FLAG_TYPE_FILE) {
		ret->st.st_mode = S_IFREG | 0444;
	} else {
		ret->st.st_mode = S_IFDIR | 0755;
	}
	ret->type = type;
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

fs_node_t* _fs_tree_add_path(fs_node_t *r, const char *path)
{
	if (path[0] == '\0') {
		return NULL;
	}
	char *p = strchr(path, '/');
	fs_node_t *r_desc;
	if (p != NULL) {
		char *dir_name = strndup(path, p-path);
		r_desc = fs_node_create(dir_name, ZIP_FILE_FLAG_TYPE_DIR);
		fs_node_add_desc(r, r_desc);
		return r_desc;
	} else {
		r_desc = fs_node_create(strdup(path), ZIP_FILE_FLAG_TYPE_FILE);
		//TODO: Fix duplicate with the if branch
		fs_node_add_desc(r, r_desc);
		return r_desc;
	}
	return _fs_tree_add_path(r_desc, p+1);
}

fs_node_t* fs_node_lookup_existing_path(fs_node_t *r, const char *path, const char **missing_path)
{
	// FIXME: Bug on missing path calculations
	char *p = strchr(path, '/');
	for (unsigned int i = 0; i < r->num_desc; ++i) {
		if (strncmp(path, r->desc[i]->name, p - path) == 0) {
			fs_node_t *ret = fs_node_lookup_existing_path(r->desc[i], p+1, missing_path);
			*missing_path = p+1;
			return ret;
		}
	}
	*missing_path = path;
	return r;
}

fs_node_t* fs_tree_add_path(fs_tree_t *r, const char *path)
{
	const char *missing_path;
	fs_node_t *root = fs_node_lookup_existing_path(r->root, path, &missing_path);
	log_debug("_fs_tree_add_path(%s, %s)", root->name, missing_path);
	return _fs_tree_add_path(root, missing_path);
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
		log_debug("Adding %s to filsystem tree", name);
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
