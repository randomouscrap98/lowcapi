# lowcapi - 2024
# Most of this was taken from chatgpt, I'm just too stupid and lazy these 
# days to go searching and remembering how to do all this

# curl - https://github.com/curl/curl

CC = gcc
CFLAGS_common = -Wall
LDFLAGS = -lcurl
PRG = lowcapi
CSVTPRG = testcsv

# -- The actual part you may need to modify if you add stuff --
BASEDEPS = lcutils.c mycsv.c api.c
SRCS = main.c $(BASEDEPS)

# -- Change this to whatever libs/include you need for your system
ifeq ($(CSYS),windows)
    CFLAGS_common = -Wall -IC:/msys64/mingw64/include -LC:/msys64/mingw64/lib -DBUILDWINDOWS
    PRG = lowcapi_win.exe
endif

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
.PHONY: clean all run windows

# First target always run, so it's "all" of course
all: $(PRG)

run: all
	./$(PRG) 2> err.txt

runtestcsv: $(CSVTPRG)
	./$^ "csvtest/files"

$(CSVTPRG): mycsv.o csvtest/test.o
	$(CC) $(CFLAGS) -o $@ $^

# Main compilation, should produce 'lowcapi'
$(PRG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

# Generic command to build all .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(PRG) $(CSVTPRG) 
	find . -name "*.o" -type f -exec rm {} +

