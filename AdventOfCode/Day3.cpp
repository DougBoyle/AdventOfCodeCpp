#include <iostream>
#include <fstream>
#include <string> // getline
#include <set>

using namespace std;


int itemPriority(char item) {
    if (item >= 'a' && item <= 'z') {
        return 1 + (item - 'a');
    }
    else {
        return 27 + (item - 'A');
    }
}

namespace day3 {

    void processLine(string& line, int& total) {
        size_t items = line.length();
        size_t compartmentSize = items / 2;
        set<char> compartment;

        for (size_t i = 0; i < compartmentSize; i++) {
            compartment.insert(line[i]);
        }

        for (size_t i = compartmentSize; i < items; i++) {
            char item = line[i];
            if (compartment.contains(item)) {
                total += itemPriority(item);
                return;
            }
        }
    }

    void part1()
    {
        ifstream input("Day3.txt");
        int total = 0;

        string line;
        while (getline(input, line)) {
            processLine(line, total);
        }

        cout << total << endl; // 7581
    }


    void part2()
    {
        ifstream input("Day3.txt");
        int total = 0;

        string line;
        set<char> sharedItems;
        set<char> items2;

        while (getline(input, line)) {
            sharedItems.clear();
            for (const char& c : line) {
                sharedItems.insert(c);
            }

            // intersect with second set
            getline(input, line);
            items2.clear();
            for (const char& c : line) {
                items2.insert(c);
            }
            
            // intersect, based on each set being sorted
            set<char>::iterator it1 = sharedItems.begin();
            set<char>::iterator it2 = items2.begin();
            while (it1 != sharedItems.end() && it2 != items2.end()) {
                if (*it1 < *it2) sharedItems.erase(it1++); // extra in sharedItems, remove
                else if (*it2 < *it1) it2++; // extra in items2, skip
                else {
                    it1++;
                    it2++;
                }
            }
            sharedItems.erase(it1, sharedItems.end()); // anything remaining is extra

            // 3rd set, look for the unique overlapping element
            getline(input, line);
            for (const char& c : line) {
                if (sharedItems.contains(c)) {
                    total += itemPriority(c);
                    break;
                }
            }
        }

        cout << total << endl; // 2525
    }


    int main() {
        part2();

        // wait to close
        cin.get();

        return 0;
    }

}