CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pthread
TARGET = cse4001_sync
SRC = main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)

