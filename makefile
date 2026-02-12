CC = gcc
SRC = main.c common.c diagnostic.c lexer.c
TARGET = bin/seal
TEST_FILES = main.seal

make:
	$(CC) $(SRC) -o $(TARGET)

run:
	./$(TARGET) $(TEST_FILES)

clean:
	rm $(TARGET)
	rm -rf $(TARGET).dSYM
