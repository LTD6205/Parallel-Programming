CXX = mpic++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Iinclude

# Định nghĩa 3 file thực thi riêng biệt
TARGET_COLOR = graph_coloring_test
TARGET_BF = bellman_ford_test
TARGET_KRUSKAL = kruskal_test

# Các file nguồn cho Graph Coloring
SRCS_COLOR = \
	src/main_coloring.cpp \
	src/graph.cpp \
	src/coloring_seq.cpp \
	src/coloring_mpi.cpp \
	src/correctness.cpp

# Các file nguồn cho Bellman-Ford
SRCS_BF = \
	src/main_bellman_ford.cpp \
	src/graph.cpp \
	src/bellman_ford_seq.cpp \
	src/bellman_ford_mpi.cpp \
	src/correctness_bellman_ford.cpp

# Các file nguồn cho Kruskal
SRCS_KRUSKAL = \
	src/main_kruskal_algo.cpp \
	src/graph.cpp \
	src/kruskal_seq.cpp \
	src/kruskal_mpi.cpp \
	src/correctness.cpp

OBJS_COLOR = $(SRCS_COLOR:.cpp=.o)
OBJS_BF = $(SRCS_BF:.cpp=.o)
OBJS_KRUSKAL = $(SRCS_KRUSKAL:.cpp=.o)

# Lệnh mặc định sẽ build cả 3 chương trình
all: $(TARGET_COLOR) $(TARGET_BF) $(TARGET_KRUSKAL)

$(TARGET_COLOR): $(OBJS_COLOR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS_COLOR)

$(TARGET_BF): $(OBJS_BF)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS_BF)

$(TARGET_KRUSKAL): $(OBJS_KRUSKAL)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS_KRUSKAL)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS_COLOR) $(OBJS_BF) $(OBJS_KRUSKAL) $(TARGET_COLOR) $(TARGET_BF) $(TARGET_KRUSKAL)

# Chạy thử Graph Coloring
run-color: $(TARGET_COLOR)
	mpirun -np 4 ./$(TARGET_COLOR) --vertices 1000 --density 0.1 --seed 42

# Chạy thử Bellman-Ford
run-bf: $(TARGET_BF)
	mpirun -np 4 ./$(TARGET_BF) --vertices 1000 --density 0.1 --seed 42 --source 0

# Chạy thử Kruskal
run-kruskal: $(TARGET_KRUSKAL)
	mpirun -np 4 ./$(TARGET_KRUSKAL) 1000

.PHONY: all clean run-color run-bf run-kruskal