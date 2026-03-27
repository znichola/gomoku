NAME = front

all: $(NAME)

init:
	@if [ "$(nvm --version)" != "$(node --version)" ]; then \
		nvm use v24.14.1; \
	fi

$(NAME): dev

# dist is a folder (./dist/)
dist: init install
	npm run build

build: dist

dev: init install
	npm run dev

install: init package.json.mklock

package.json.mklock: package.json
	@cat $< > $@
	npm install

clean:
	rm -rf dist

fclean: clean
	rm package.json.mklock

# not useful to fclean with re in npm project
re: clean all

.PHONY: all init $(NAME) build dev install clean fclean re

