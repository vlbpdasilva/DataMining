839project: main.o helper.o
	g++ -std=c++11 -g main.o helper.o -o 839project

main.o: main.cpp
	g++ -std=c++11 -g -c main.cpp

helper.o: helper.cpp
	g++ -std=c++11 -g -c helper.cpp
	
clean: 
	rm *.o 839project
