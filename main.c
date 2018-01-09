#define FUSE_USE_VERSION 31

#include <zip.h>

#include <fuse.h>

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <stddef.h>

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

#include <collectc/array.h>

#include "log.h"
#include "fs_tree.h"

fs_tree_t g_fs_tree;
zip_t *g_zip;

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
	ArrayIter it;
	array_iter_init(&it, p->desc);
	fs_node_t *node;
	while (array_iter_next(&it, (void**)&node) == CC_OK) {
		filler(buffer, node->name, NULL, 0, 0);
	}
	return 0;
}

static int cfs_open(const char *path, struct fuse_file_info *fi)
{
	(void) path;
	(void) fi;
	return 0;
}

static int cfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	(void) fi;
	const char *archive_path = path+1;
	zip_file_t *f = zip_fopen(g_zip, archive_path, 0);
	if (f == NULL) {
		log_error("zip_fopen %s: %s", archive_path, zip_strerror(g_zip));
		return -1;
	}
	zip_fseek(f, offset, SEEK_SET);
	int ret = zip_fread(f, buf, size);
	zip_fclose(f);
	return ret;
}

static struct fuse_operations ffs_oper = {
    .getattr 	= cfs_getattr,
    .readdir	= cfs_readdir,
	.open		= cfs_open,
	.read		= cfs_read,
};

void _fs_tree_debug_print(fs_node_t *r, int depth)
{
	for (int i = 0; i < depth; ++i) {
		printf("\t");
	}
	printf("-> %s\n", r->name);
	for (unsigned int i = 0; i < array_size(r->desc); ++i) {
		fs_node_t *node;
		array_get_at(r->desc, i, (void*)&node);
		_fs_tree_debug_print(node, depth+1);
	}
}

void fs_tree_debug_print(fs_tree_t *r)
{
	_fs_tree_debug_print(r->root, 0);
}

static struct options {
	const char *filename;
	int show_help;
} options;

#define OPTION(t, p) { t, offsetof(struct options, p), 1 }

static const struct fuse_opt option_spec[] = {
	OPTION("--name=%s", filename),
	OPTION("-h", show_help),
	OPTION("--help", show_help),
	FUSE_OPT_END
};

static void show_help(const char *progname) 
{
	printf("usage: %s --name=<filename> <mountpoint>\n\n", progname);
}

int main(int argc, char** argv)
{
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1) {
		log_error("fuse_opt_parse error");
		return EXIT_FAILURE;
	}
	if (options.show_help) {
		show_help(argv[0]);
		assert(fuse_opt_add_arg(&args, "--help") == 0);
		args.argv[0] = (char*)"";
	}
	if (options.filename == NULL) {
		fprintf(stderr, "Invalid number of arguments\n");
		show_help(argv[0]);
		return EXIT_FAILURE;
	}
	g_zip = zip_open(options.filename, ZIP_RDONLY, NULL);
	g_fs_tree = build_fs_tree_from_zip(g_zip);
	return fuse_main(args.argc, args.argv, &ffs_oper, NULL);
}

