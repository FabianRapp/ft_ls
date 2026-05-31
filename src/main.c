#include <libft.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


//-l     use a long listing format
//       -R, --recursive //list subdirectories recursively
//-a, --all //do not ignore entries starting with .
//-t     sort by time, newest first; see --time
//-r, --reverse //reverse order while sorting
typedef enum {
	PATH,
	LONG,
	RECURSIVE,
	ALL,
	REVERSE,
} t_args_type;

typedef struct s_modes {
	unsigned Long : 1;
	unsigned recursive : 1;
	unsigned all : 1;
	unsigned reverse : 1;
} t_modes;

const char *args_type_to_str(t_args_type type) {
	switch (type) {
		default:
			case (LONG): return "LONG";
			case (RECURSIVE): return "RECURSIVE";
			case (ALL): return "ALL";
			case (REVERSE): return "REVERSE";
			case (PATH): return "PATH";
	}
}

void set_mode(t_modes *modes, t_args_type type) {
	switch (type) {
		default:
			case (LONG): modes->Long = 1; break ;
			case (RECURSIVE): modes->recursive = 1; break ;
			case (ALL): modes->all = 1; break ;
			case (REVERSE): modes->reverse = 1; break ;
			case (PATH): break ;
	}
}

t_args_type args_type(const char *arg) {
	const char *flags[][2] = {
		{"-l", "-l"},
		{"-R", "--recursive"},
		{"-a", "--all"},
		{"-r", "--reverse"},
	};
	const t_args_type types[] = {
		LONG, RECURSIVE, ALL, REVERSE,
	};
	for (int i = 0; i < sizeof flags / sizeof flags[0]; i++) {
		if (!ft_strcmp(arg, flags[i][0]) || !ft_strcmp(arg, flags[i][1])) {
			return types[i];
		}
	}
	return PATH;
}

void print_info(const char *path, t_modes modes) {
	printf("%s ", path);
}

char *get_sub_dir_path(const char *path, const char *sub_name) {
	char *sub_dir;
	int path_len = ft_strlen(path);
	if (path[path_len - 1] == '/') {
		sub_dir = ft_strjoin(path, sub_name);
	} else {
		char *tmp = ft_strjoin(path, "/");
		if (!tmp) {
			ft_fprintf(2, "Malloc error\n");
			return NULL;
		}
		sub_dir = ft_strjoin(tmp, sub_name);
		free(tmp);
	}
	if (!sub_dir) {
		ft_fprintf(2, "Malloc error\n");
		return NULL;
	}
	return sub_dir;
}

//todo
void handle_dir(const char *path, t_modes modes, char ***paths, int *path_count) {
	DIR *dir = opendir(path);
	if (dir == NULL) {
		switch (errno) {
			case (EACCES): break ;// Permission denied/.
			case (EBADF): break ;//  fd is not a valid file descriptor opened for reading.
			case (EMFILE): break ;// The per-process limit on the number of open file descriptors has been reached.
			case (ENFILE): break ;// The system-wide limit on the total number of open files has been reached.
			case (ENOENT): break ;// Directory does not exist, or name is an empty string.
			case (ENOMEM): break ;// Insufficient memory to complete the operation.
			case (ENOTDIR): assert(0);//name is not a directory.
		}
	}
	if (modes.recursive) {
		ft_printf("%s:\n", path);
	}
	//todo: first save all the paths, sort them case sensitive and then work on them
	for (struct dirent *content = readdir(dir); content != NULL; content = readdir(dir)) {
		if (content->d_name[0] == '.' && !modes.all) {
			continue ;
		}
		print_info(content->d_name, modes);
		if (modes.recursive && content->d_type == DT_DIR
			&& ft_strcmp(content->d_name, ".") && ft_strcmp(content->d_name, "..")) {
			char *rec_dir = get_sub_dir_path(path, content->d_name);
			if (!rec_dir) {
				return ;
			}
			dyn_arr_add_save((void**)paths, (void*)(&rec_dir), (*path_count)++);
		}
	}
	closedir(dir);
	if (modes.recursive) {
		ft_printf("\n");
	}
}

void handle_path(const char *path, t_modes modes, char ***paths, int *path_count) {
	struct stat stat;
	if (lstat(path, &stat) < 0) {
		// todo: error
		ft_fprintf(2, "Error with path: %s: %s\n", path, strerror(errno));
		return ;
	}
	unsigned type_bits = stat.st_mode & S_IFMT;

    if (type_bits == S_IFSOCK) { //  0140000   socket
	}
	if (type_bits == S_IFLNK) { //symbolic link
	}
	if (type_bits == S_IFREG) { //regular file
	}
	if (type_bits == S_IFBLK) { //block device
	}
	if (type_bits == S_IFDIR) { //directory
		handle_dir(path, modes, paths, path_count);
	}
	if (type_bits == S_IFCHR) { //character device
	}
	if (type_bits == S_IFIFO) { //FIFO
	}

	unsigned permisson_bits = stat.st_mode & 0777;

mode_t    st_mode;        /* File type and mode */
nlink_t   st_nlink;       /* Number of hard links */
uid_t     st_uid;         /* User ID of owner */
gid_t     st_gid;         /* Group ID of owner */
off_t     st_size;        /* Total size, in bytes */
blksize_t st_blksize;     /* Block size for filesystem I/O */
blkcnt_t  st_blocks;      /* Number of 512B blocks allocated */
}

int main(int ac, char **av) {
	char **paths = dyn_arr_init(sizeof(char *), 0);
	if (!paths) {
		ft_printf("Malloc error\n");
		return 1;
	}
	int path_count = 0;
	t_modes modes = {0};
	const char *cur_dir = ".";

	for (int i = 1; i < ac; i++) {
		const t_args_type arg_type = args_type(av[i]);
		set_mode(&modes, arg_type);
		if (arg_type == PATH) {
			dyn_arr_add_save((void**)(&paths), (void*)(av + i), path_count++);
		}
		ft_printf("args type %s\n", args_type_to_str(arg_type));
	}

	//ft_printf("path count: %d\n", path_count);

	if (path_count == 0) {
		dyn_arr_add_save((void**)(&paths), (void*)(&cur_dir), path_count++);
	}
	const int non_recurisve_path_count = path_count;
	for (int i = 0 ; i < path_count; i++) {
		handle_path(paths[i], modes, &paths, &path_count);
	}
	for (int i = non_recurisve_path_count; i < path_count; i++) {
		free(paths[i]);
	}

	dyn_arr_free((void **)(&paths));
	if (modes.recursive) {
		ft_printf("\n");
	}
	return 0;
}
