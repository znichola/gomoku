# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: znichola <znichola@student.42lausanne.ch>  +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/10/25 20:59:50 by znichola          #+#    #+#              #
#    Updated: 2025/10/25 20:59:50 by znichola         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME	= gomoku

CC		= clang++
CFLAGS	= -Wall -Wextra -Werror
CFLAGS	+= -std=c++20

ifdef DEBUG
CFLAGS	+= -g3 -fsanitize=address
endif

FILES	= main Server

OBJS_PATH = obj/
SRCS_PATH = src/
INCS_PATH = -Iinclude/.

SRCS	= $(addprefix $(SRCS_PATH), $(addsuffix .cpp, $(FILES)))
OBJS	= $(addprefix $(OBJS_PATH), $(addsuffix .o, $(FILES)))

all	: $(NAME) deps/elm

$(OBJS_PATH)%.o: $(SRCS_PATH)%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $(INCS_PATH) -o $@ $<

$(NAME)	: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

clean	:
	$(RM) $(OBJS)

fclean	: clean
	$(RM) $(NAME)

re	: fclean all

deps/elm:
	mkdir -p deps && cd deps \
	&& curl -L -o elm.gz https://github.com/elm/compiler/releases/download/0.19.1/binary-for-linux-64-bit.gz \
	&& gzip -d elm.gz \
	&& chmod +x elm


run : all
	./$(NAME)

.PHONY: all clean fclean re run

