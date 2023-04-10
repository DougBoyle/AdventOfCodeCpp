#include <iostream>
#include <fstream>
#include <string> // getline
#include <vector>
#include <algorithm> // sort
#include <map>

using namespace std;

enum RPS { rock, paper, scissors };

map<char, RPS> decodeRPS{
    {'A', rock}, {'B', paper}, {'C', scissors },
    {'X', rock}, {'Y', paper}, {'Z', scissors },
};

int RpsResult(RPS& ours, RPS& theirs) {
    if (ours == theirs) return 3;
    switch (ours) {
    case rock:
        return theirs == scissors ? 6 : 0;
    case paper:
        return theirs == rock ? 6 : 0;
    case scissors:
        return theirs == paper ? 6 : 0;
    }
}

int RpsValue(RPS& value) {
    switch (value) {
    case rock:
        return 1;
    case paper:
        return 2;
    case scissors:
        return 3;
    }
}

RPS decodePart2(RPS& theirs, char outcome) {
    switch (outcome) {
    case 'X':
        return theirs == rock ? scissors :
            theirs == scissors ? paper : rock;
    case 'Y':
        return theirs;
    case 'Z':
        return theirs == rock ? paper :
            theirs == scissors ? rock : scissors;
    }
}

namespace day2 {


    void part1()
    {
        ifstream input("Day2.txt");
        int total = 0;

        string line;
        // getline returns the passed input stream,
        // io evaulates to true as long as neither failbit or badbit set, get updated by reading (e.g. EOF)
        while (getline(input, line)) {
            // array-style decodeRPS[key] does a default/zero insert if not already present, rather than throwing. 
            // Instead use decodeRPS.at(key)
            RPS theirs = decodeRPS.at(line[0]);
            RPS mine = decodeRPS.at(line[2]);
            total += RpsResult(mine, theirs) + RpsValue(mine);
        }

        cout << total << endl; // 14264
    }

    void part2()
    {
        ifstream input("Day2.txt");
        int total = 0;

        string line;
        // getline returns the passed input stream,
        // io evaulates to true as long as neither failbit or badbit set, get updated by reading (e.g. EOF)
        while (getline(input, line)) {
            // array-style decodeRPS[key] does a default/zero insert if not already present, rather than throwing. 
            // Instead use decodeRPS.at(key)
            RPS theirs = decodeRPS.at(line[0]);
            RPS mine = decodePart2(theirs, line[2]);
            total += RpsResult(mine, theirs) + RpsValue(mine);
        }

        cout << total << endl; // 12382
    }


    int main() {
        part2();

        // wait to close
        cin.get();

        return 0;
    }

}