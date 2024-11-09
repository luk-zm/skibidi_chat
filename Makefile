# Install
BIN = skibidi-chat

# Flags
CFLAGS += -g -std=c99 -Wall -Wextra -pedantic # -O2

SRC = main.c
OBJ = $(SRC:.c=.o)

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	LIBS = -lSDL2 -framework OpenGL -lm -lGLEW
else
	LIBS = -lSDL2 -lGL -lm -lGLU -lGLEW
endif

$(BIN):
	@mkdir -p bin
	rm -f bin/$(BIN) $(OBJS)
	$(CC) $(SRC) $(CFLAGS) -o bin/$(BIN) $(LIBS)
