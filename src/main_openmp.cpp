#include <vector>
#include <iostream>

using namespace std;

using Board=vector<vector<bool>>;

const int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
const int dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};

Board oneIteration(const Board& b) {
    Board newBoard{b};

    #pragma omp parallel for
    for (size_t i = 0; i < b.size(); ++i) {
        #pragma omp parallel for
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

	    if (numLiveCells < 2 || numLiveCells > 3) newBoard[i][j] = false;
	    else if (numLiveCells == 3) newBoard[i][j] = true;
	}
    }

    return newBoard;
}

string to_string(const Board& b) {
   string s = "";
   for (const vector<bool>& line: b) {
      for (const bool& cell: line) {
	  s += cell ? "1 " : "0 ";
      }
      s += "\n";
   }
   return s;
}

int main() {
   size_t M, numRows, numCols;

   cin >> numRows >> numCols >> M;

   Board b(numRows, vector<bool>(numCols));
   for (size_t i = 0; i < M; ++i) {
      size_t x, y;
      cin >> x >> y;
      b[x][y] = true;
   }

   size_t numIterations = 0;

   cin >> numIterations;
   cout << "Initial matrix:\n" << to_string(b) << "\n";
   for (size_t i = 0; i < numIterations; ++i) {
      b = std::move(oneIteration(b));
      cout << "Iteration " << (i + 1) << "\n" << to_string(b) <<"\n";

   }

   return 0;
}


