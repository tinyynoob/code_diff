cc = gcc
ccflags = -pthread

.PHONY: run format

run: code_edit_distance
	./code_edit_distance old.txt new.txt

format: code_edit_distance.c ptxt.c ptxt.h
	clang-format -i $^

code_edit_distance: code_edit_distance.c ptxt.c
	$(cc) -o $@ $^ $(ccflags)

