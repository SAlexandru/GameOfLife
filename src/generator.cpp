#include <chrono>
#include <string>
#include <random>
#include <iostream>
#include <fstream>


using namespace std;


int main(int argc, char **argv) {
    if (5 != argc) {
        cout << argv[0] << " [fileName] [numRows] [numCols] [bernoulli_distribution p -- between 0 and 1]\n";
        return 0;
    }

    std::random_device rd;
    mt19937 rng(rd());
    int numRows = stoi(argv[2]), numCols = stoi(argv[3]);
    bernoulli_distribution bernoulli(stod(argv[4]));
    uniform_int_distribution<> dis(10, 100000);
    vector<pair<int, int>> idx;

    ofstream out{argv[1]};
    
    out << numRows << ' ' << numCols << ' ';
    for (int i = 0; i < numRows; ++i) {
        for (int j = 0; j < numCols; ++j) {
            if (bernoulli(rng)) {
                idx.emplace_back(i, j);
            }
        }
    }
    out  << idx.size() << '\n';
    for (const auto& x: idx) {
        out << x.first << ' ' << x.second << '\n';
    }

    out << dis(rng) << '\n';
    out.close();
    return 0;
}


