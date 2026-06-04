#include <libft.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

/*
 * todo:
 * [ ] Malloc checks for dyn_arr ops
 * [ ] LONG
 * [ ] multiple flags in 1 args
 * [ ] Generell error handling ..
 * [ ] TIME sorting
 * [x] line splitting for multiple input paths, currently only for -R
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
	TIME,
} t_args_type;

typedef struct s_modes {
	unsigned Long : 1;
	unsigned recursive : 1;
	unsigned all : 1;
	unsigned reverse : 1;
	unsigned time : 1;
} t_modes;

struct my_dir_ent {
	struct dirent dirent;
	struct stat stats;
};

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
			case (TIME): modes->time = 1; break ;
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
		{"-t", "-t"},
	};
	const t_args_type types[] = {
		LONG, RECURSIVE, ALL, REVERSE, TIME,
	};
	for (int i = 0; i < sizeof flags / sizeof flags[0]; i++) {
		if (!ft_strcmp(arg, flags[i][0]) || !ft_strcmp(arg, flags[i][1])) {
			return types[i];
		}
	}
	return PATH;
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

//todo: long format
//todo: needs to take max size of the files in the dir
void print_info(const char *name, struct stat stats, t_modes modes, const char *path, size_t max_file_size_in_dir) {
	if (!modes.Long) {
		ft_printf("%s  ", name);
		return ;
	}
	char *file_path = get_sub_dir_path(path, name);
	if (!file_path) {
		//todo: error etc..
		return ;
	}

	char *permissons = "---------";
	char *owner = "OWNER";
	char *flags = "FLAGS";
	char *group = "GROUP";



	//todo: missing link stuff
	ft_printf("%s %u %s %s %u", permissons, stats.st_nlink, owner, group, stats.st_size);
	//stats.st_atim


	{ // time
		char *last_modified = ctime(&stats.st_mtim);
		while (*last_modified != ' ') {
			last_modified++;
		}
		int time_len = ft_strlen(last_modified);
		time_len -= 9; // -9: removes: newline, year spaces, column, seconds
		write(1, last_modified, time_len);
	}

	ft_printf(" %s", name);
	ft_printf("\n");
	free(file_path);
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
	struct my_dir_ent *a_ent = (struct my_dir_ent *)a;
	struct my_dir_ent *b_ent = (struct my_dir_ent *)b;
	return ft_strcmp(a_ent->dirent.d_name, b_ent->dirent.d_name) > 0;
}

bool cmp_dir_entry_reverse(const void *a, const void *b) {
	struct my_dir_ent *a_ent = (struct my_dir_ent *)a;
	struct my_dir_ent *b_ent = (struct my_dir_ent *)b;
	return ft_strcmp(a_ent->dirent.d_name, b_ent->dirent.d_name) < 0;
}

bool cmp_dir_entry_time(const void *a, const void *b) {
	struct my_dir_ent *a_ent = (struct my_dir_ent *)a;
	struct my_dir_ent *b_ent = (struct my_dir_ent *)b;
	if (a_ent->stats.st_mtim.tv_sec == b_ent->stats.st_mtim.tv_sec) {
		return a_ent->stats.st_mtim.tv_nsec < b_ent->stats.st_mtim.tv_nsec;
	}
	return a_ent->stats.st_mtim.tv_sec < b_ent->stats.st_mtim.tv_sec;
}

bool cmp_dir_entry_time_reverse(const void *a, const void *b) {
	struct my_dir_ent *a_ent = (struct my_dir_ent *)a;
	struct my_dir_ent *b_ent = (struct my_dir_ent *)b;
	if (a_ent->stats.st_mtim.tv_sec == b_ent->stats.st_mtim.tv_sec) {
		return a_ent->stats.st_mtim.tv_nsec > b_ent->stats.st_mtim.tv_nsec;
	}
	return a_ent->stats.st_mtim.tv_sec > b_ent->stats.st_mtim.tv_sec;
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
	struct my_dir_ent* sub_files = (struct my_dir_ent*)dyn_arr_init(sizeof(struct my_dir_ent), 24);

	int sub_file_count = 0;
	size_t max_file_size_in_dir = 0; //todo: needs to be filles for -l
	for (struct dirent *content = readdir(dir); content != NULL; content = readdir(dir)) {

		/*Even though struct dirent is of a certain sice, it is not guaranteed
		 * that the full struct is allocated. For short file names the allocation
		 * might be smaller, which leads to segaults in dyn_arr_add_save. */
		struct my_dir_ent local;
		ft_memcpy(&local.dirent, content, content->d_reclen);
		if (modes.Long || modes.time) {
			char *sub_file_path = get_sub_dir_path(path, local.dirent.d_name);
			if (!sub_file_path) {
				//todo: error etc..
				return;
			}
			assert(lstat(sub_file_path, &local.stats) == 0);
			free(sub_file_path);
		}

		if (dyn_arr_add_save((void**)(&sub_files), (void*)(&local), sub_file_count++)) {
			ft_fprintf(2, "Malloc Error\n");
			closedir(dir);
			return ;
		}
	}
	closedir(dir);
	if (modes.reverse && !modes.time) {
		ft_sort(sub_files, sizeof(struct my_dir_ent), sub_file_count, cmp_dir_entry_reverse);
	} else if (!modes.reverse && modes.time) {
		ft_sort(sub_files, sizeof(struct my_dir_ent), sub_file_count, cmp_dir_entry_time);
	} else if (modes.reverse && modes.time) {
		ft_sort(sub_files, sizeof(struct my_dir_ent), sub_file_count, cmp_dir_entry_time_reverse);
	} else /*(!modes.reverse && !modes.time) */ {
		ft_sort(sub_files, sizeof(struct my_dir_ent), sub_file_count, cmp_dir_entry);
	}
	for (int i = 0; i < sub_file_count; i++) {
		struct my_dir_ent *content = sub_files + i;
		if (content->dirent.d_name[0] == '.' && !modes.all) {
			continue ;
		}
		print_info(content->dirent.d_name, content->stats, modes, path, max_file_size_in_dir);
		if (modes.recursive && content->dirent.d_type == DT_DIR
			&& ft_strcmp(content->dirent.d_name, ".") && ft_strcmp(content->dirent.d_name, "..")) {
			char *rec_dir = get_sub_dir_path(path, content->dirent.d_name);
			if (!rec_dir) {
				//todo: error etc..
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
		// todo: needs time flag sorting
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
