#include <iostream>
#include <fstream>
#include <string> // getline
#include <deque>
#include <map>
#include <stdexcept>

using namespace std;

namespace day6 {

    void findMarker(size_t size) {
        map<char, size_t> lastPosition;
        ifstream input("Day6.txt");

        char c;

        size_t start = 0;
        size_t end = 0;

        while (end - start < size) {
            if (!(input >> c)) {
                throw invalid_argument("reached end of input");
            }
            auto position = lastPosition.find(c);
            if (position != lastPosition.end() && position->second >= start) {
                start = position->second + 1;
            }
            lastPosition[c] = end;
            end = input.tellg();
        }

        cout << end << endl;
    }

    void part1() {
        findMarker(4); // 1538 
    }

    void part2() {
        findMarker(14); // 2315
    }

    int main() {
        part2();

        return 0;
    }

}