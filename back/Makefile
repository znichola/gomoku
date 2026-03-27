NAME	  = gomoku

CXX       = clang++
CXXFLAGS  = -Wall -Wextra -Werror
CXXFLAGS += -std=c++20

FILES     = Server Routes GameState Board Grid Cell
MFILES    = main $(FILES)

OBJ_PATH  = obj/
GOBJ_PATH = gobj/
SRC_PATH  = src/
INC_PATH  = include/

SRC       = $(addprefix $(SRC_PATH), $(addsuffix .cpp, $(MFILES)))
HDEP      = $(addprefix $(INC_PATH), $(addsuffix .hpp, $(FILES)))
OBJ       = $(addprefix $(OBJ_PATH), $(addsuffix .o, $(MFILES)))

GOBJ      = $(addprefix $(GOBJ_PATH), $(addsuffix .o, $(MFILES)))

LIBS      = -I$(INC_PATH)

all	: $(NAME)

$(OBJ_PATH)%.o: $(SRC_PATH)%.cpp $(HDEP)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(LIBS) -c $< -o $@

$(GOBJ_PATH)%.o: $(SRC_PATH)%.cpp $(HDEP)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(LIBS) -c $< -o $@

$(NAME): $(OBJ) $(HDEP)
	$(CXX) $(CXXFLAGS) $(LIBS) -g3 -o $(NAME) $(OBJ)

# Usage: make debug && lldb gomoku_debug -o run
$(NAME)_debug: $(GOBJ) $(HDEP)
	$(CXX) $(CXXFLAGS) $(LIBS) -g -o $(NAME)_debug $(GOBJ)

$(NAME)_sanitize: $(SRC) $(HDEP)
	$(CXX) $(CXXFLAGS) $(LIBS) -fsanitize=address -g -o $(NAME)_sanitize $(GOBJ)

$(NAME)_sanitize3: $(SRC) $(HDEP)
	$(CXX) $(CXXFLAGS) $(LIBS) -fsanitize=address -g3 -o $(NAME)_sanitize $(SRC)

debug: $(NAME)_debug

sanitize: $(NAME)_sanitize

sanitize3: $(NAME)_sanitize3

clean:
	$(RM) $(OBJ_PATH)main.o $(OBJ) $(GOBJ) $(TEST_OBJ)

fclean: clean
	$(RM) $(NAME) $(NAME)_debug $(NAME)_sanitize $(NAME)_sanitize3

re: fclean all

run: all
	./$(NAME)


# Tests

TEST_DIR       = test/
TEST_OBJ_PATH = test/$(OBJ_PATH)
TEST_FILES     = moves

TEST_OBJ = $(addprefix $(TEST_DIR)obj/, $(addsuffix .o, $(TEST_FILES)))
TEST_BINS = $(TEST_FILES)

$(TEST_OBJ_PATH)%.o: $(TEST_DIR)%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -I$(INC_PATH) -o $@ $<

test/moves: $(TEST_OBJ_PATH)moves.o $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@


.PHONY: all debug sanitize sanitize3 clean fclean re run

