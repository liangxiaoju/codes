all:
	g++ -static -DNDEBUG -O4 -Wall -o ELEEYE.EXE ../base/pipe.cpp ucci.cpp pregen.cpp position.cpp genmoves.cpp hash.cpp book.cpp movesort.cpp preeval.cpp evaluate.cpp search.cpp eleeye.cpp

lib.so:
	g++ -DNDEBUG -O4 -Wall -fPIC -shared -o libeleeye.so ../base/pipe.cpp ucci.cpp pregen.cpp position.cpp genmoves.cpp hash.cpp book.cpp movesort.cpp preeval.cpp evaluate.cpp search.cpp eleeye.cpp

lib.a:
	g++ -DNDEBUG -O4 -Wall -c ../base/pipe.cpp ucci.cpp pregen.cpp position.cpp genmoves.cpp hash.cpp book.cpp movesort.cpp preeval.cpp evaluate.cpp search.cpp eleeye.cpp
	ar -r -o libeleeye.a pipe.o ucci.o pregen.o position.o genmoves.o hash.o book.o movesort.o preeval.o evaluate.o search.o eleeye.o
	rm pipe.o ucci.o pregen.o position.o genmoves.o hash.o book.o movesort.o preeval.o evaluate.o search.o eleeye.o
