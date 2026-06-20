CXX = mpic++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Iinclude

TARGET = graph_coloring_test

SRCS = \
    src/main_coloring.cpp \
    src/graph.cpp \
    src/coloring_seq.cpp \
    src/coloring_mpi.cpp \
    src/correctness.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run-local: $(TARGET)
	mpirun -np 4 ./$(TARGET) --vertices 1000 --density 0.1 --seed 42

.PHONY: all clean run-local