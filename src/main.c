#include <libft.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/*
 * todo:
 * [ ] Malloc checks for dyn_arr ops
 * [ ] LONG
 * [ ] multiple flags in 1 args
 * [ ] Generell error handling ..
 * [ ] line splitting for multiple input paths, currently only for -R
*/


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

//todo: needs rework
t_args_type args_type(const char *arg) {
	if (arg[0] != '-') {
		return PATH;
	}
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

//todo: long format
void print_info(const char *path, t_modes modes) {
	ft_printf("%s  ", path);
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

bool cmp_paths(const void *a, const void *b) {
	char **a_ent = (char **)a;
	char **b_ent = (char **)b;
	return ft_strcmp(*a_ent, *b_ent) > 0;
}

bool cmp_paths_reverse(const void *a, const void *b) {
	char **a_ent = (char **)a;
	char **b_ent = (char **)b;
	return ft_strcmp(*a_ent, *b_ent) < 0;
}

bool cmp_dir_entry(const void *a, const void *b) {
	struct dirent *a_ent = (struct dirent *)a;
	struct dirent *b_ent = (struct dirent *)b;
	return ft_strcmp(a_ent->d_name, b_ent->d_name) > 0;
}

bool cmp_dir_entry_reverse(const void *a, const void *b) {
	struct dirent *a_ent = (struct dirent *)a;
	struct dirent *b_ent = (struct dirent *)b;
	return ft_strcmp(a_ent->d_name, b_ent->d_name) < 0;
}

void handle_dir(const char *path, t_modes modes, char ***paths, int *path_count) {
	DIR *dir = opendir(path);
	if (dir == NULL) {
		ft_fprintf(2, "ft_ls: cannot open directory '%s': %s\n", path, strerror(errno));
		return ;
	}
	if (modes.recursive || *path_count > 1) {
		ft_printf("%s:\n", path);
	}
	struct dirent *sub_files = (struct dirent *)dyn_arr_init(sizeof(struct dirent), 24);
	int sub_file_count = 0;
	for (struct dirent *content = readdir(dir); content != NULL; content = readdir(dir)) {

		/*Even though struct dirent is of a certain sice, it is not guaranteed
		 * that the full struct is allocated. For short file names the allocation
		 * might be smaller, which leads to segaults in dyn_arr_add_save. */
		struct dirent local = {0};
		ft_memcpy(&local, content, content->d_reclen);

		if (dyn_arr_add_save((void**)(&sub_files), (void*)(&local), sub_file_count++)) {
			ft_fprintf(2, "Malloc Error\n");
			closedir(dir);
			return ;
		}
	}
	closedir(dir);

	if (!modes.reverse) {
		ft_sort(sub_files, sizeof(struct dirent), sub_file_count, cmp_dir_entry);
	} else {
		ft_sort(sub_files, sizeof(struct dirent), sub_file_count, cmp_dir_entry_reverse);
	}
	for (int i = 0; i < sub_file_count; i++) {
		struct dirent *content = sub_files + i;
		if (content->d_name[0] == '.' && !modes.all) {
			continue ;
		}
		print_info(content->d_name, modes);
		if (modes.recursive && content->d_type == DT_DIR
			&& ft_strcmp(content->d_name, ".") && ft_strcmp(content->d_name, "..")) {
			char *rec_dir = get_sub_dir_path(path, content->d_name);
			if (!rec_dir) {
				dyn_arr_free((void**)(&sub_files));
				return ;
			}
			dyn_arr_add_save((void**)paths, (void*)(&rec_dir), (*path_count)++);
		}
	}
	dyn_arr_free((void**)(&sub_files));
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

	for (int i = 1; i < ac; i++) {
		const t_args_type arg_type = args_type(av[i]);
		set_mode(&modes, arg_type);
		if (arg_type == PATH) {
			char *path = ft_strdup(av[i]);
			if (!path) {
				//todo: cleanup
				ft_printf("Malloc error\n");
				return 1;
			}
			dyn_arr_add_save((void**)(&paths), (void*)(&path), path_count++);
		}
		//ft_printf("args type %s\n", args_type_to_str(arg_type));
	}

	//ft_printf("path count: %d\n", path_count);

	if (path_count == 0) {
		char *cur_dir = ft_strdup(".");
		if (!cur_dir) {
			//todo: cleanup
			ft_printf("Malloc error\n");
			return 1;
		}
		dyn_arr_add_save((void**)(&paths), (void*)(&cur_dir), path_count++);
	}
	int i =0;
	while (i < path_count) {
		// todo: super slow to sort all the time, better lists with sorted insert
		if (!modes.reverse) {
			ft_sort(paths + i, sizeof(char *), path_count - i, cmp_paths);
		} else {
			ft_sort(paths + i, sizeof(char *), path_count - i, cmp_paths_reverse);
		}
		handle_path(paths[i], modes, &paths, &path_count);
		if (++i < path_count) {
			ft_printf("\n\n");
		}
	}
	for (int i = 0; i < path_count; i++) {
		free(paths[i]);
	}

	dyn_arr_free((void **)(&paths));
	return 0;
}
