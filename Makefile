NAME := ft_ls

CC := gcc
#-fsanitize=address 
CFLAGS := -g -fsanitize=address 
LD_FLAGS := -fsanitize=address
LIBFT := ./libft/libft.a

SOURCE = src/main.c

INCLUDE := -Ilibft

.PHONY: clean fclean re


$(NAME): $(SOURCE) $(LIBFT)
	$(CC) $(CFLAGS) $(SOURCE) $(INCLUDE) $(LIBFT) -o $(NAME) $(LD_FLAGS)

$(LIBFT):
	cd libft && make "CFLAGS=$(CFLAGS)"


clean:
	cd libft && make clean
	rm -f $(NAME)

fclean: clean
	rm -rf obj
	cd libft && make fclean

re: fclean $(NAME)
