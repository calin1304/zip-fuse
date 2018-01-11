#include "fs_node.h"

#include "defines.h"

fs_node_t* fs_node_create(char *name, int type)
{
	fs_node_t *ret = malloc(sizeof(fs_node_t));
	memset(ret, 0, sizeof(fs_node_t));
	ret->name = name;
	zip_stat_init(&ret->fstat);
	memset(&(ret->st), 0, sizeof(struct stat));
	fs_node_set_type(ret, type);
	array_new(&ret->desc);
	return ret;
}

void fs_node_free(fs_node_t *r)
{
	ArrayIter it;
	array_iter_init(&it, r->desc);
	fs_node_t *p;
	while (array_iter_next(&it, (void**)&p) == CC_OK) {
		fs_node_free(p);
	};
	array_destroy_cb(r->desc, free);
}

void fs_node_set_type(fs_node_t *r, int type)
{
	if (type == ZIP_FILE_FLAG_TYPE_FILE) {
		r->st.st_mode = S_IFREG | 0444;
	} else {
		r->st.st_mode = S_IFDIR | 0755;
	}
	r->type = type;
}

struct fs_node* fs_node_find_desc_n(struct fs_node *r, const char *name, size_t sz)
{
	if (sz == 0) {
		return r;
	}
	ArrayIter it;
	array_iter_init(&it, r->desc);
	fs_node_t *p;
	while (array_iter_next(&it, (void**)&p) == CC_OK) {
		if (strncmp(name, p->name, sz) == 0) {
			return p;
		}
	}
	return NULL;
}

fs_node_t* fs_node_find_desc(fs_node_t *r, const char *name)
{
	return fs_node_find_desc_n(r, name, strlen(name));
}