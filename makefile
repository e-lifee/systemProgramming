CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj

all: tarsau

tarsau: $(OBJ_DIR)/main.o $(OBJ_DIR)/createArchive.o $(OBJ_DIR)/extractArchive.o $(OBJ_DIR)/isTextFile.o
	$(CC) $(CFLAGS) -o tarsau $(OBJ_DIR)/main.o $(OBJ_DIR)/createArchive.o $(OBJ_DIR)/extractArchive.o $(OBJ_DIR)/isTextFile.o

$(OBJ_DIR)/main.o: $(SRC_DIR)/main.c $(INC_DIR)/createArchive.h $(INC_DIR)/extractArchive.h $(INC_DIR)/fileInfo.h $(INC_DIR)/textfile.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(OBJ_DIR)/createArchive.o: $(SRC_DIR)/createArchive.c $(INC_DIR)/createArchive.h $(INC_DIR)/fileInfo.h $(INC_DIR)/textfile.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(OBJ_DIR)/extractArchive.o: $(SRC_DIR)/extractArchive.c $(INC_DIR)/extractArchive.h $(INC_DIR)/fileInfo.h $(INC_DIR)/textfile.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(OBJ_DIR)/isTextFile.o: $(SRC_DIR)/isTextFile.c $(INC_DIR)/textfile.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -f tarsau $(OBJ_DIR)/*.o

.PHONY: run
run: tarsau
	./tarsau

