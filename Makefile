SOURCES :=                              \
	src/pancake/PancakeInstance.cpp     \
	src/pancake/PancakeState.cpp        \
	src/Search.cpp                      \
	src/tiles/GluedTiles.cpp            \
	src/tiles/ManhattanDistance.cpp     \
	src/tiles/Tiles.cpp                 \
	src/tiles/TilesState.cpp

CXX := g++
CXXFLAGS := -Wall -Wextra -Wno-unused-parameter -O3 -DCACHE_NODE_F_VALUE -DNDEBUG
CXXINCLUDE := -Isrc -Iboost_1_49_0


.PHONY: all search doc clean clean_all

search: boost_1_49_0
	$(CXX) $(CXXFLAGS) $(SOURCES) $(CXXINCLUDE) -o search

boost_1_49_0: boost_1_49_0.tar.bz2
	tar xjf boost_1_49_0.tar.bz2 && cd boost_1_49_0 && ./bootstrap.sh && ./b2

boost_1_49_0.tar.bz2:
	wget http://sourceforge.net/projects/boost/files/boost/1.49.0/boost_1_49_0.tar.bz2

all: search doc

doc:
	doxygen doxygen.conf

clean:
	rm -rf build doc
	rm -f search

clean_all: clean
	rm -rf boost_1_49_0
