NAME := ircserv

CXX := c++

CXXFLAGS := -Werror -Wextra -Wall -std=c++20 -I./includes

SRCDIR := src
# Add source files here as you create them
SRCS := $(SRCDIR)/main.cpp $(SRCDIR)/Server.cpp $(SRCDIR)/Client.cpp $(SRCDIR)//commands/channel_join.cpp $(SRCDIR)/Server_utils.cpp $(SRCDIR)/commands/auth_utils.cpp \
	$(SRCDIR)/commands/auth.cpp $(SRCDIR)/Channel.cpp $(SRCDIR)/commands/channel_topic.cpp $(SRCDIR)/commands/privmsg.cpp


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
