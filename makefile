winRestore: main.o resource.res
	g++ build/main.o build/resource.res -o bin/winRestore

main.o: src/main.cpp
	g++ -c src/main.cpp -o build/main.o

resource.res: resources/resource.rc
	windres -i resources/resource.rc -O coff -o build/resource.res