NAME = gomoku

all: $(NAME)

$(NAME): back front

back/gomoku:
	make -C back gomoku

front/dist: 
	make -C front build

back: back/gomoku

front: front/dist

dev: back
	make -C front dev

clean:
	make -C back clean
	make -C front clean

fclean: clean
	make -C back fclean
	make -C front fclean

re: fclean all

.PHONY: all $(NAME) back front dev clean fclean re
