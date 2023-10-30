# lowcapi - 2023
# Most of this was taken from chatgpt, I'm just too stupid and lazy these 
# days to go searching and remembering how to do all this

CC = gcc
CFLAGS = -Wall -Ideps
LDFLAGS = -lncurses
PRG = lowcapi

SRCS = main.c deps/toml.c
OBJS = $(SRCS:.c=.o)

# chatgpt told me that .PHONY is there to ensure the command always runs, even
# if there's a file named as such. But I think it got it wrong when it didn't 
# include 'all' in the list, see https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
.PHONY: clean all

# First target always run, so it's "all" of course
all: $(PRG)

# Main compilation, should produce 'lowcapi'
$(PRG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) # $(LDFLAGS)

# Generic command to build all .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<


clean:
	rm -f $(PRG) $(OBJS)
