#include <libft.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>


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

void handle_dir(char *path) {
	DIR *dir = opendir(paths[i]);
	if (dir == NULL) {
		switch (errno) {
			case (EACCES): break ;// Permission denied.
			case (EBADF): break ;//  fd is not a valid file descriptor opened for reading.
			case (EMFILE): break ;// The per-process limit on the number of open file descriptors has been reached.
			case (ENFILE): break ;// The system-wide limit on the total number of open files has been reached.
			case (ENOENT): break ;// Directory does not exist, or name is an empty string.
			case (ENOMEM): break ;// Insufficient memory to complete the operation.
			case (ENOTDIR): assert(0);//name is not a directory.
			}
		}
		closedir(dir);
}

int main(int ac, char **av) {
	char **paths =dyn_arr_init(sizeof(char *), 0);
	int path_count = 0;
	t_modes modes = {0};
	ft_printf("hello world\n");
	for (int i = 1; i < ac; i++) {
		const t_args_type arg_type = args_type(av[i]);
		set_mode(&modes, arg_type);
		if (arg_type == PATH) {
			dyn_arr_add_save((void**)(&paths), (void*)(av + i), path_count++);
		}
		ft_printf("args type %s\n", args_type_to_str(arg_type));
	}
	ft_printf("path count: %d\n", path_count);
	for (int i = 0 ; i < path_count; i++) {
		ft_printf("Working on path %s..\n", paths[i]);


	}

	dyn_arr_free((void **)(&paths));
	return 0;
}
