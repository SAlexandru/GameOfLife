#include <array>
#include <deque>
#include <iostream>
#include <fstream>
#include <chrono>
#include <boost/mpi.hpp>
#include <boost/mpi/collectives.hpp>
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/serialization/deque.hpp>

using namespace std;

namespace mpi = boost::mpi;

constexpr int root = 0; // root process
constexpr int MAX_SIZE = 50000;

using Board = deque<deque<bool>>;

const int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
const int dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};

string to_string(const Board& b) {
   string s = "";
   for (const deque<bool>& line: b) {
      for (const bool& cell: line) {
	  s += cell ? "1 " : "0 ";
      }
      s += "\n";
   }
   return s;
}

Board transpose(const Board& b) {
    if (b.empty()) {
      return deque<deque<bool>> (0, deque<bool>(0));
    }

    Board bt(b[0].size(), deque<bool>(b.size()));

    for (size_t i = 0; i < b.size(); ++i) {
        for (size_t j = 0; j < b[i].size(); ++j) {
            bt[j][i] = b[i][j];
        }
    }

    return bt;
}


Board partition(const Board& b, size_t start, size_t size ) {
    if (size == 0) size = b.size();
    int stop = std::min<size_t>(b.size(), start + size);

    Board partitiondB;
    partitiondB.resize(size);
    
    for (int i = start; i < stop; ++i) {
        partitiondB[i - start] = (b[i]);
    }

    return partitiondB;
}

Board partition(const Board& b, size_t start) {
    Board partitiondB;
    
    for (size_t i = start; i < b.size(); ++i) {
        partitiondB.push_back(b[i]);
    }

    return partitiondB;
}

void doProcessing(Board& b, size_t start, size_t end) {
    Board newB{b};
    for (size_t i = start; i < end; ++i) {
        for (size_t j = 0; j < b[i].size(); ++j) {
	    int numLiveCells = 0;
	    int numDeadCells = 0;

	    for (int k = 0; k < 8; ++k) {
		int newI = i + dx[k];
		int newJ = j + dy[k];

		if (newI >= 0 && newJ >= 0 && newI < b.size() && newJ < b[i].size()) {
		    numLiveCells += b[newI][newJ] ? 1 : 0;
		    numDeadCells += b[newI][newJ] ? 0 : 1;
		}
	    }

	    if (numLiveCells < 2 || numLiveCells > 3) newB[i][j] = false;
	    else if (numLiveCells == 3) newB[i][j] = true;
        }
    }
    b = std::move(newB);
}

deque<bool> todequeOfBool (bool v[], size_t N) {
    deque<bool> b(N);
    for (size_t i = 0; i < N; ++i) {
        b[i] = v[i];
    }
    return b;
}

void doForFirstProcess(mpi::communicator& world, Board& b) {
    deque<bool> lastRow;
    mpi::request reqs;
    reqs = world.isend(1, root + 1, b.back());
    world.recv(1, root, lastRow);

    b.push_back(lastRow);

    doProcessing(b, 0, b.size() - 1);

    b.pop_back();
}

void doForLastProcess(mpi::communicator& world, Board& b, int rank) {
    deque<bool> firstRow;
    mpi::request reqs;
    reqs = world.isend(rank - 1, rank - 1, b[0]);
    world.recv(rank - 1, rank, firstRow);

    b.push_front(std::move(firstRow));

    doProcessing(b, 1, b.size());

    b.pop_front();
}

void doForInBetweenProcess(mpi::communicator& world, Board& b, int rank) {
    deque<bool> firstRow, lastRow;
    mpi::request reqs[2];

    reqs[0] = world.isend(rank - 1, rank - 1, b[0]);
    reqs[1] = world.isend(rank + 1, rank + 1, b.back());
    world.recv(rank - 1, rank, firstRow);
    world.recv(rank + 1, rank, lastRow);

    b.push_front(std::move(firstRow));
    b.push_back(std::move(lastRow));

    doProcessing(b, 1, b.size() - 1);

    b.pop_front();
    b.pop_back();
}


int main() {
   mpi::environment env;
   mpi::communicator world;

    auto start = std::chrono::steady_clock::now();

   Board local;
   size_t M;
   
   if (root == world.rank()) {
       size_t numRows, numCols, x, y;
       cin >> numRows >> numCols >> M;
       Board b;
       b.resize(numRows);
       for (size_t i = 0; i < numRows; ++i) {
           b[i].resize(numCols);
       }
       for (size_t i = 0; i < M; ++i) {
           cin >> x >> y;
           b[x][y] = true;
       }

       const size_t numProcess = std::min<size_t>(numRows, world.size());
       const size_t sizeOfAPartition = numRows / numProcess;

       local = partition(b, 0, sizeOfAPartition);

       for (size_t i = 1; i < numProcess - 1; ++i) {
           world.send(i, root, partition(b, i * sizeOfAPartition, sizeOfAPartition));
       }
       world.send(numProcess - 1, root, partition (b, (numProcess - 1) * sizeOfAPartition));
   }
   else {
       world.recv(root, root, local);
   }

   broadcast(world, M, root); 

   world.barrier();

   for (size_t i = 0; i < M; ++i) {
       if (world.rank() == 0) doForFirstProcess(world, local);
       else if (world.rank() == world.size() - 1) doForLastProcess(world, local, world.rank());
       else doForInBetweenProcess(world, local, world.rank());
   }

   world.barrier();

   string result;
   reduce(world, to_string(local), result, [](const string& s, const string& bAsS) { return s + bAsS; }, root);

   if (world.rank() == root) {
       cout << result ;
   }

   auto duration = std::chrono::duration_cast<std::chrono::milliseconds> 
                                   (std::chrono::steady_clock::now() - start);

   result.clear();
   string t = to_string(local.size()) + " " + to_string(local[0].size()) + " " + to_string(M) + " " + to_string(world.rank()) + " " + to_string(duration.count()) + "  milliseconds\n";
   reduce(world, t, result, [] (string s, string t) {return s + t;}, root);

   if (world.rank() == root) {
       cout << result;
   }

   return 0;
}


