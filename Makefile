all: main
main:
		gcc -O2 -lm bmphide.c main.c -o bmphide
debug:
		gcc -ggdb3 -lm bmphide.c main.c -o bmphide

test:
	./bmphide -i picture.bmp -o picture2.bmp -s text
	./bmphide -i picture2.bmp > text.retrieved
	wc -c text
	md5sum text
	md5sum text.retrieved


