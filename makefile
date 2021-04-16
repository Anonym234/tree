BIN=tree
DEPLOY_PATH=~/.bin/
FILES=tree.c
PARAMETERS=..

CC=gcc
FLAGS=-std=c18 -Wall -Wpedantic
DEPLOY_EXTRA_FLAGS=-O3

run: compile
	./$(BIN) $(PARAMETERS)

compile:
	$(CC) $(FLAGS) $(FILES) -o $(BIN)

deploy: clean
	$(CC) $(FLAGS) $(DEPLOY_EXTRA_FLAGS) $(FILES) -o $(DEPLOY_PATH)$(BIN)

clean:
	$(RM) $(BIN)
