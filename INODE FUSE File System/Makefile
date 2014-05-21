all:	3600fs 3600mkfs test3600fs

3600fs: 3600fs.c
	gcc $< disk.c -std=gnu99 -lfuse -O0 -g -lm -Wall -pedantic -Wextra -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26 -o $@

3600mkfs: 3600mkfs.c
	gcc $< disk.c -std=gnu99 -O0 -g -lm -Wall -pedantic -Wextra -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26 -o $@

test3600fs: test3600fs.c
	gcc $< disk.c -std=gnu99 -O0 -g -lm -Wall -pedantic -Wextra -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26 -o $@

test: 3600fs 3600mkfs
	./test

ourtests: 3600fs 3600mkfs test3600fs
	./test3600fs

clean:	
	rm -f 3600fs 3600mkfs fs-log.txt log.txt test3600fs

squeaky: clean
	rm -f MYDISK
