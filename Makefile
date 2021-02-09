all: lib/libparser.a shared/libparser.so

#################### lib.a ####################
lib/libparser.a: bin/parser.o
	ar cr libparser.a bin/parser.o
	mv libparser.a lib/

################### lib.so ###################
shared/libparser.so: bin/parser-fpic.o
	gcc -g -shared -o libparser.so bin/parser-fpic.o -I.
	mv libparser.so shared/

################## bin/static ##################
bin/parser.o:
	gcc -c -g source/parser.c -o parser.o -I.
	mv parser.o bin/
bin/buffer.o:
	gcc -c -g source/buffer.c -o buffer.o -I.
	mv buffer.o bin/

################## bin/shared ##################
bin/buffer-fpic.o:
	gcc -c -g source/buffer.c -fPIC -o buffer-fpic.o -I.
	mv buffer-fpic.o bin/
bin/parser-fpic.o:
	gcc -c -g source/parser.c -fPIC -o parser-fpic.o -I.
	mv parser-fpic.o bin/

#################### clean #####################
clean:
	rm -rf bin/*.o
	rm -rf lib/*.a
	rm -rf shared/*.so