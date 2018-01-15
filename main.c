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

static void cfs_destroy(void *private_data)
{
	(void) private_data;

	fs_tree_free(&g_fs_tree);
	zip_close(g_zip);
}

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
	if (p->type != ZIP_FILE_FLAG_TYPE_DIR) {
		return -ENOTDIR;
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
	if (zip_name_locate(g_zip, path+1, 0) == -1) {
		return -ENOENT;
	}
	if ((fi->flags & O_ACCMODE) != O_RDONLY) {
		return -EACCES;
	}
	zip_file_t *f = zip_fopen(g_zip, path + 1, 0);
	if (f == NULL) {
		log_error("zip_fopen %s: %s", path+1, zip_strerror(g_zip));
		return -1;
	}
	fi->fh = (uint64_t)f;
	return 0;
}

static int cfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	zip_file_t *f = (zip_file_t*)(fi->fh);
	zip_fseek(f, offset, SEEK_SET);
	int ret = zip_fread(f, buf, size);
	return ret;
}

static int cfs_release(const char *path, struct fuse_file_info *fi)
{
	zip_file_t *f = (zip_file_t*)(fi->fh);
	zip_fclose(f);
	return 0;
}

static struct fuse_operations ffs_oper = {
	.destroy	= cfs_destroy,
    .getattr 	= cfs_getattr,
    .readdir	= cfs_readdir,
	.open		= cfs_open,
	.read		= cfs_read,
	.release	= cfs_release,
};

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
	int errorp;
	if ((g_zip = zip_open(options.filename, ZIP_RDONLY, &errorp)) == NULL) {
		zip_error_t ze;
		zip_error_init_with_code(&ze, errorp);
		log_fatal("zip_open: %s", zip_error_strerror(&ze));
		zip_error_fini(&ze);
		return EXIT_FAILURE;
	}
	g_fs_tree = build_fs_tree_from_zip(g_zip);
	return fuse_main(args.argc, args.argv, &ffs_oper, NULL);
}

