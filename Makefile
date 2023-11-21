shell: main.o task1.o task2.o task3.o task4.o
	g++ -std=c++14 -w main.o task1.o task2.o task3.o task4.o -o shell

main.o: main.cpp
	g++ -std=c++14 -w -c main.cpp

task1.o: task1.cpp task1.h
	g++ -std=c++14 -w -c task1.cpp

task2.o: task2.cpp task2.h
	g++ -std=c++14 -w -c task2.cpp

task3.o: task3.cpp task3.h
	g++ -std=c++14 -w -c task3.cpp

task4.o: task4.cpp task4.h
	g++ -std=c++14 -w -c task4.cpp

clean:
	rm -rf *.o shell