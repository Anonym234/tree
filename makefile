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
	# gcc -Wall -Wpedantic -std=c18 tree.c -o tree
	$(CC) $(FLAGS) $(FILES) -o $(BIN)

deploy:
	$(CC) $(FLAGS) $(DEPLOY_EXTRA_FLAGS) $(FILES) -o $(DEPLOY_PATH)$(BIN)
