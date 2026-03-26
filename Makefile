all : back/gomoku front/dist

init:
	@if [ "$(nvm --version)" != "$(node --version)" ]; then \
		nvm use v24.14.1; \
	fi

back/gomoku:
	make -C back/

front/node_modules: init
	npm --prefix front install

front/dist: init front/node_modules
	npm --prefix front run build

clean:
	make -C back/ clean
	rm -rf front/dist

fclean: clean
	make -C back/ fclean

re: fclean all