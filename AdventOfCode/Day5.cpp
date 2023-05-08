#include <iostream>
#include <fstream>
#include <string> // getline
#include <vector>
#include <deque>
#include <algorithm>

using namespace std;

// deque used rather than stacks, to allow building in reverse order
void ensureStacks(vector<deque<char>>& stacks, int n) {
    while (stacks.size() <= n) { // or equal as we're 1-indexing
        stacks.push_back(deque<char>());
    }
}

namespace day5 {

    void move1(deque<char>& from, deque<char>& to, int n) {
        while (n-- > 0) {
            to.push_back(from.back()); from.pop_back();
        }
    }

    // Uses a global working stack to effectively reverse the order twice
    // Could be more efficient moving between vectors/arrays directly (can limit how much new allocation done), but not too bad (still O(1)).
    deque<char> workingSpace;
    void move2(deque<char>& from, deque<char>& to, int n) {
        while (n-- > 0) {
            workingSpace.push_back(from.back()); from.pop_back();
        }
        while (workingSpace.size() > 0) {
            to.push_back(workingSpace.back()); workingSpace.pop_back();
        }
    }

    template <typename Move>
    void run(Move&& move)
    {
        ifstream input("Day5.txt");

        // first read in the starting shape
        vector<deque<char>> stacks;

        string line;
        while (getline(input, line)) {
            if (line.length() == 0) break; // second part
            for (int i = 0; i < line.length(); i += 4) {
                if (line[i] == '[') {
                    // each stack plus padding has a width of 4, columns indexed from 1
                    int idx = i / 4 + 1;
                    ensureStacks(stacks, idx);
                    stacks[idx].push_front(line[i + 1]);
                    
                }
            }
        }

        // each line now looks like:
        //  'move N from A to B'
        int n, fromCol, toCol;
        while (input.ignore(strlen("move ")) >> n) {
            input.ignore(strlen(" from "));
            input >> fromCol;
            input.ignore(strlen(" to "));
            input >> toCol;

            deque<char>& from = stacks[fromCol];
            deque<char>& to = stacks[toCol];
       
             move(from, to, n);
        }
        
        // first stack is a dummy to allow 1-indexing
        for_each(++stacks.begin(), stacks.end(), [](deque<char> stack) { cout << stack.back(); });

        cout << endl; // part1 = TLFGBZHCN, part2 = QRQFHFWCL
    }


    int main() {
        run(move2);

        return 0;
    }

}