#auto vars
#$^ : target dependencies
#$@ : target name

#name vars
NAME=hash
BIN=$(NAME)2
HEAD=j$(NAME)

#command vars
CC=gcc
CFLAGS=-g -Wall
c = $(CC) $(CFLAGS)
RM=rm
RFLAGS=-f
r = $(RM) $(RFLAGS)

all: $(BIN)

#pattern rule (w/ header precompile)
%.o: %.c %.h $(HEAD).h
	$(c) -c $^

$(BIN): $(BIN).o
	$(c) -o $@ $^

clean:
	$(r) a.out *.o *.gch

purge: clean
	$(r) $(BIN)
