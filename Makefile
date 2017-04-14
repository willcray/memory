CXX = g++ -std=c++11
CXXFLAGS  = -Wall -g
OFILES = main.o
CXXFILES = main.cpp

program: $(OFILES)
	$(CXX) $(CXXFLAGS) $(OFILES) -o memory

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp