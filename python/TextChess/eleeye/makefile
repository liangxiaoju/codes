all: x86 arm

x86:
	g++ -static -DNDEBUG -O4 -Wall -o ELEEYE.PC ../base/pipe.cpp ucci.cpp pregen.cpp position.cpp genmoves.cpp hash.cpp book.cpp movesort.cpp preeval.cpp evaluate.cpp search.cpp eleeye.cpp
arm:
	arm-linux-gnueabi-g++ -static -DNDEBUG -O4 -Wall -o ELEEYE.ARM ../base/pipe.cpp ucci.cpp pregen.cpp position.cpp genmoves.cpp hash.cpp book.cpp movesort.cpp preeval.cpp evaluate.cpp search.cpp eleeye.cpp
