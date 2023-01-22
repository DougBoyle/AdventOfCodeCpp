#include <iostream>
#include <fstream>
#include <string> // getline
#include <vector>
#include <algorithm> // sort

using namespace std;


int elfTotal = 0;
int maxElfTotal = 0;

void elfFinished() {
    if (elfTotal > maxElfTotal) maxElfTotal = elfTotal;
    elfTotal = 0;
}

void part1()
{
    ifstream input("Day1.txt");

    string line;
    // getline returns the passed input stream,
    // ios evaulates to true as long as neither failbit or badbit set, get updated by reading (e.g. EOF)
    while (getline(input, line)) {
        if (line.length() == 0) elfFinished();
        else elfTotal += stoi(line); // stoi = string-to-int
    }
    elfFinished();

    cout << maxElfTotal << endl; // 71471
}




vector<int> elves;

void elfFinished2() {
    elves.push_back(elfTotal);
    elfTotal = 0;
}

void part2()
{
    ifstream input("Day1.txt");

    string line;
    while (getline(input, line)) {
        if (line.length() == 0) elfFinished2();
        else elfTotal += stoi(line);
    }
    elfFinished2();

    // now sort to get top 3
    // sort takes 2 "Random access iterators" for range to sort, plus comparator to get descending order
    // (alternative solution is to provide 'reverse' iterators rbegin and rend
    sort(elves.begin(), elves.end(), greater<>());

    // C++ does implicit conversion, will happily work out how to combine ints/strings
    cout << elves[0] << " " << elves[1] << " " << elves[2] << endl; // 71471 70523 69195
    cout << elves[0] + elves[1] + elves[2] << endl; // 211189
}

int main() {
    part2();

    // wait to close
    cin.get();

    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
