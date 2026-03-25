all : back/gomoku front/dist

back/gomoku:
	make -C back/

front/node_modules:
	npm --prefix front install

front/dist: front/node_modules
	npm --prefix front run build

clean:
	make -C back/ clean
	rm -rf front/dist

fclean: clean
	make -C back/ fclean

re: fclean all