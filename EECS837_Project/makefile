miner: 837project.o lem.o
	g++ -std=c++11 -g 837project.o lem.o -o miner

837project.o: 837project.cpp
	g++ -std=c++11 -g -c 837project.cpp

lem.o: lem.cpp
	g++ -std=c++11 -g -c lem.cpp
	
clean: 
	rm *.o miner