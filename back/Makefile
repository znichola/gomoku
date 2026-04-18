NAME     ?= gomoku
PORT     ?= 9012
# make NAME=gomoku_9013 PORT=9013

CXX       = clang++
CXXFLAGS  = -Wall -Wextra -Werror
CXXFLAGS += -std=c++20

FILES     = Board Cell GameState Grid Routes Server Utils MessageQueue AI Vector2D
# CXXFILES are files without header (.hpp file) :
CXXFILES  = RoutesArbiter main_$(PORT) $(FILES)

OBJ_PATH  = obj/
GOBJ_PATH = gobj/
SRC_PATH  = src/
INC_PATH  = include/

SRC       = $(addprefix $(SRC_PATH), $(addsuffix .cpp, $(CXXFILES)))
HDEP      = $(addprefix $(INC_PATH), $(addsuffix .hpp, $(FILES)))
OBJ       = $(addprefix $(OBJ_PATH), $(addsuffix .o, $(CXXFILES)))

GOBJ      = $(addprefix $(GOBJ_PATH), $(addsuffix .o, $(CXXFILES)))

LIBS      = -I$(INC_PATH)

all: $(NAME)

$(OBJ_PATH)main_$(PORT).o: $(SRC_PATH)main.cpp $(HDEP)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -D PORT=$(PORT) $(LIBS) -c $< -o $@

$(GOBJ_PATH)main_$(PORT).o: $(SRC_PATH)main.cpp $(HDEP)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -D PORT=$(PORT) $(LIBS) -c $< -o $@

$(SRC_PATH)main_$(PORT).cpp: $(SRC_PATH)main.cpp $(HDEP)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -D PORT=$(PORT) $(LIBS) -c $< -o $@

$(OBJ_PATH)%.o: $(SRC_PATH)%.cpp $(HDEP)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(LIBS) -c $< -o $@

$(GOBJ_PATH)%.o: $(SRC_PATH)%.cpp $(HDEP)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(LIBS) -g -c $< -o $@

$(NAME): $(OBJ) $(HDEP)
	$(CXX) $(CXXFLAGS) $(LIBS) -o $(NAME) $(OBJ)

# Usage: make debug && lldb gomoku_debug -o run
$(NAME)_debug: $(GOBJ) $(HDEP)
	$(CXX) $(CXXFLAGS) $(LIBS) -g -o $(NAME)_debug $(GOBJ)

$(NAME)_sanitize: $(GOBJ) $(HDEP)
	$(CXX) $(CXXFLAGS) $(LIBS) -fsanitize=address -g -o $(NAME)_sanitize $(GOBJ)

$(NAME)_sanitize3: $(SRC) $(HDEP)
	$(CXX) $(CXXFLAGS) $(LIBS) -fsanitize=address -g3 -o $(NAME)_sanitize $(SRC)

debug: $(NAME)_debug

watcher:
	@(trap '' INT; \
	while true; do \
		echo "\033[0;32m=== BUILD ===\033[0;00m"; \
		if ! make debug PORT=$(PORT); then \
			echo "\n\033[0;31mBuild failed.\033[0;00m"; \
			read -p "Press enter to retry (q to quit): " input; \
			[ "$$input" = "q" ] && exit 0; \
			continue; \
		fi; \
		echo "\033[0;32m=== RUN ===\033[0;00m"; \
		./$(NAME)_debug || true; \
		echo ""; \
		read -p "Press enter to continue (q to quit): " input; \
		[ "$$input" = "q" ] && break; \
	done)

sanitize: $(NAME)_sanitize

sanitize3: $(NAME)_sanitize3

clean:
	$(RM) $(OBJ_PATH)main.o $(OBJ) $(GOBJ) $(TEST_OBJ)

bclean:
	$(RM) $(NAME) $(NAME)_debug $(NAME)_sanitize $(NAME)_sanitize3 $(NAME)_9*

fclean: clean bclean

re: fclean all

run: all
	./$(NAME)


# Tests

TEST_DIR       = test/
TEST_OBJ_PATH = test/$(OBJ_PATH)
TEST_FILES     = moves

TEST_OBJ = $(addprefix $(TEST_DIR)obj/, $(addsuffix .o, $(TEST_FILES)))
TEST_MAIN_OBJ  = $(addprefix $(OBJ_PATH), $(addsuffix .o, $(FILES)))
TEST_BINS = $(TEST_FILES)

$(TEST_OBJ_PATH)%.o: $(TEST_DIR)%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -I$(INC_PATH) -o $@ $<

test/moves: $(TEST_OBJ_PATH)moves.o $(TEST_MAIN_OBJ)
	$(CXX) $(CXXFLAGS) $() $^ -o $@


.PHONY: all debug watcher sanitize sanitize3 clean bclean fclean re run

include $(DEP)
