NAME	= gomoku

CC		= clang++
CFLAGS	= -Wall -Wextra -Werror
CFLAGS	+= -std=c++20

ifdef DEBUG
CFLAGS	+= -g3 -fsanitize=address
endif

FILES	= Server Routes GameState Board Grid 

OBJS_PATH = obj/
SRCS_PATH = src/
INCS_PATH = include/.

SRCS	= $(addprefix $(SRCS_PATH), $(addsuffix .cpp, $(FILES)))
OBJS	= $(addprefix $(OBJS_PATH), $(addsuffix .o, $(FILES)))

all	: $(NAME)

$(OBJS_PATH)%.o: $(SRCS_PATH)%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -I$(INCS_PATH) $< -o $@

$(NAME)	: $(OBJS) $(OBJS_PATH)main.o
	$(CC) $(CFLAGS) $^ -o $@

clean	:
	$(RM) $(OBJS) $(OBJS_PATH)main.o

fclean	: clean
	$(RM) $(NAME)

re	: fclean all

run : all
	./$(NAME)


# Tests

TEST_DIR       = test/
TEST_OBJS_PATH = test/$(OBJS_PATH)
TEST_FILES     = moves

TEST_OBJS = $(addprefix $(TEST_DIR)obj/, $(addsuffix .o, $(TEST_FILES)))
TEST_BINS = $(TEST_FILES)

$(TEST_OBJS_PATH)%.o: $(TEST_DIR)%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -I$(INCS_PATH) -o $@ $<

test/moves: $(TEST_OBJS_PATH)moves.o $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@


.PHONY: all clean fclean re run

