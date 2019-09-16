CFLAGS =  -g -march=native -Ofast -Wextra -Wall -DSOUND -DCGB
TARGET = emu
LDFLAGS =
# if on windows need extra flags
ifeq ($(OS),Windows_NT)
	LDFLAGS += -lmingw32 -lSDL2main -lSDL2
else 
	LDFLAGS = -lSDL2
endif
CC = gcc
OBJDIR=obj
DIRS = src

CFILES = $(foreach DIR,$(DIRS),$(wildcard $(DIR)/*.c))
COBJFILES = $(CFILES:%.c=$(OBJDIR)/%.o)

$(TARGET): $(COBJFILES)
	$(CC) $(CFLAGS) -flto  $(COBJFILES) -o $(TARGET) $(LDFLAGS)

$(COBJFILES): $(OBJDIR)/%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
