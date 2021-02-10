all: c-parser-a c-parser-so
#################### main ####################
c-parser-a: lib/libparser.a main.o
	gcc -g -o c-parser-a main.o -I. -Llib/ -lparser
#
c-parser-so: shared/libparser.so main.o
	gcc -g -o c-parser-so  main.o -Lshared/ -lparser -Wl,-rpath,shared/ -I.
#
main.o:
	gcc -g -c main.c -o main.o -I.
#################### lib.a ####################
lib/libparser.a: bin/parser.o bin/cleaner.o bin/buffer.o
	ar cr libparser.a bin/parser.o bin/cleaner.o bin/buffer.o
	mv libparser.a lib/

################### lib.so ###################
shared/libparser.so: bin/parser-fpic.o bin/cleaner-fpic.o bin/buffer-fpic.o
	gcc -g -shared -o libparser.so bin/cleaner-fpic.o bin/buffer-fpic.o bin/parser-fpic.o -I.
	mv libparser.so shared/

################## bin/static ##################
bin/parser.o:
	gcc -c -g source/parser.c -o parser.o -I.
	mv parser.o bin/
bin/buffer.o:
	gcc -c -g source/buffer.c -o buffer.o -I.
	mv buffer.o bin/
bin/cleaner.o:
	gcc -c -g source/cleaner.c -o cleaner.o -I.
	mv cleaner.o bin/
################## bin/shared ##################
bin/buffer-fpic.o:
	gcc -c -g source/buffer.c -fPIC -o buffer-fpic.o -I.
	mv buffer-fpic.o bin/
bin/cleaner-fpic.o:
	gcc -c -g source/cleaner.c -fPIC -o cleaner-fpic.o -I.
	mv cleaner-fpic.o bin/
bin/parser-fpic.o:
	gcc -c -g source/parser.c -fPIC -o parser-fpic.o -I.
	mv parser-fpic.o bin/
#################### clean #####################
clean:
	rm-rf *.o
clean-o:
	rm -rf bin/*.o
clean-a:
	rm -rf lib/*.a
clean-so:
	rm -rf shared/*.so
