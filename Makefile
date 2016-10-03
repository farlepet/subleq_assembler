# Makefile for subleq_assembler

MAINDIR	= $(CURDIR)
SRC	= $(MAINDIR)/src
INC	= $(MAINDIR)/inc

CSRC	= $(wildcard $(SRC)/*.c)
OBJ	= $(patsubst %.c,%.o,$(CSRC))
EXEC	= $(MAINDIR)/subleq_assembler


CFLAGS  = -Wall -Wextra -Werror -I $(INC)
LDFLAGS = 

all: $(OBJ)
	@echo -e "\033[33m  \033[1mLD\033[21m    \033[34m$(EXEC)\033[0m"
	@$(CC) $(OBJ) $(LDFLAGS) -o $(EXEC)

clean:
	@echo -e "\033[33m  \033[1mCleaning subleq_assembler\033[0m"
	@rm -f $(OBJ) $(EXEC)

%.o: %.c
	@echo -e "\033[32m  \033[1mCC\033[21m    \033[34m$<\033[0m"
	@$(CC) $(CFLAGS) -c -o $@ $<
