all: ls find tree

ls: ls.c
	gcc -g -Wall -Wvla -fsanitize=address -o ls ls.c

find: find.c
	gcc -g -Wall -Wvla -fsanitize=address -o find find.c

tree: tree.c
	gcc -g -Wall -Wvla -fsanitize=address -o tree tree.c

clean:
	rm ls find tree
