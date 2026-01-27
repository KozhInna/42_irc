NAME := ircserv

CXX := c++

CXXFLAGS := -Werror -Wextra -Wall -std=c++17 -I./includes

SRCDIR := src
# Add source files here as you create them
SRCS := $(SRCDIR)/main.cpp $(SRCDIR)/Server.cpp $(SRCDIR)/Client.cpp $(SRCDIR)/Server_utils.cpp
        # $(SRCDIR)/Channel.cpp
        # $(SRCDIR)/commands/auth.cpp 

OBJDIR := obj
OBJS := $(SRCS:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
DEPS := $(OBJS:.o=.d)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
