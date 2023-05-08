#include <iostream>
#include <fstream>
#include <string> // getline
#include <set>

using namespace std;


istream& readVals(ifstream& input, int& l1, int& l2, int& r1, int& r2) {
    char sep;
    // e.g. 1-3,2-4
    return input
        >> l1 >> sep >> r1
        >> sep
        >> l2 >> sep >> r2;
}

// Pred&& is a forward/universal referenece, already a deduced type, so takes an lvalue reference/rvalue reference (& or &&) as approriate.
template<typename Pred> int countPredicate(ifstream& input, Pred&& p) {
    int l1, l2, r1, r2;
    int total = 0;
    while (readVals(input, l1, l2, r1, r2)) {
        total += p(l1, l2, r1, r2);
    }
    return total;
}

namespace day4 {


    void part1()
    {
        ifstream input("Day4.txt");

        int total = countPredicate(input, [](int l1, int l2, int r1, int r2) { return l1 <= l2 && r1 >= r2 || l2 <= l1 && r2 >= r1; });

        cout << total << endl; // 424
    }

    void part2()
    {
        ifstream input("Day4.txt");


        int total = countPredicate(input, [](int l1, int l2, int r1, int r2) { return l2 <= r1 && l1 <= r2; });

        cout << total << endl; // 804
    }


    int main() {
        part2();

        return 0;
    }

}