# lowcapi - 2023
# Most of this was taken from chatgpt, I'm just too stupid and lazy these 
# days to go searching and remembering how to do all this

CC = gcc
CFLAGS_common = -Wall -Ideps
LDFLAGS = -lncurses -lcurl
PRG = lowcapi

# -- The actual part you may need to modify if you add stuff --
SRCS = main.c config.c api.c deps/toml.c deps/csv.c deps/log.c
OBJS = $(SRCS:.c=.o)


CONFIG = release

ifeq ($(CONFIG),debug)
    CFLAGS = $(CFLAGS_common) -g -DDEBUG
else
    CFLAGS = $(CFLAGS_common) -O2
endif


# chatgpt told me that .PHONY is there to ensure the command always runs, even
# if there's a file named as such. But I think it got it wrong when it didn't 
# include 'all' in the list, see https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
.PHONY: clean all run

# First target always run, so it's "all" of course
all: $(PRG)

run: all
	./$(PRG)

# Main compilation, should produce 'lowcapi'
$(PRG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

# Generic command to build all .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(PRG) $(OBJS)
