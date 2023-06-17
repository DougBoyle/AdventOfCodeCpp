#include <iostream>
#include <fstream>
#include <string> // getline
#include <sstream> // string -> stream
#include <queue>
#include <vector>
#include <cassert>

using namespace std;

enum Op { ADD, MULT, SQUARE };

class Monkey {
    const Op op;
    const int value;
    const int divTest;
    const int caseTrueMonkey;
    const int caseFalseMonkey;

    int inspected = 0;
    queue<int> items;
    vector<Monkey>& monkeys;

private:
    void push(int val) {
        items.push(val);
    }

    int newVal(int val) {
        if (op == ADD) {
            return (val + value) / 3;
        }
        else if (op == MULT) {
            return (val * value) / 3;
        }
        else if (op == SQUARE) {
            return (val * val) / 3;
        }
        else {
            string errorMsg = "Unknown operation " + op;
            throw invalid_argument(errorMsg);
        }
    }

    void forward(int val) {
        if (val % divTest == 0) {
            monkeys[caseTrueMonkey].push(val);
        }
        else {
            monkeys[caseFalseMonkey].push(val);
        }
    }

public:
    Monkey(const Op op, const int value, const int divTest, const int caseTrueMonkey, const int caseFalseMonkey, queue<int>& items, vector<Monkey>& monkeys)
        : op(op), value(value), divTest(divTest), caseTrueMonkey(caseTrueMonkey), caseFalseMonkey(caseFalseMonkey), items(items), monkeys(monkeys) {}


    void inspect() {
        while (!items.empty()) {
            inspected++;
            int val = items.front(); items.pop();
            int updated = newVal(val);
            forward(updated);
        }
    }

    int getInspected() {
        return inspected;
    }
};

istringstream getUnindentedLine(ifstream& input) {
    string line;
    getline(input, line);
    istringstream lineStream = istringstream(line);
    lineStream >> ws;
    return lineStream;
}

namespace day11 {
    void part1() {
        ifstream input("Day11.txt");

        vector<Monkey> monkeys;

        string line;
        string placeholderWord;
        int monkeyIdx;

        while (getline(input, line)) {

            // First line, index of monkey (not strictly necessary)
            istringstream lineStream(line);
            lineStream.ignore(strlen("Monkey ")) >> monkeyIdx;

            // Second line: '   Starting items: 62, 92, 50, 63, 62, 93, 73, 50'
            lineStream = getUnindentedLine(input);
            lineStream.ignore(strlen("Starting items: "));

            queue<int> items;
            int item;
            char c;
            while (lineStream >> item) {
                items.push(item);
                lineStream >> c; // skip ',' delimeter
            }

            // Third line: '   Operation: new = old * 7'
            lineStream = getUnindentedLine(input);
            lineStream.ignore(strlen("Operation: new = old "));
            char opChar;
            int val;
            lineStream >> opChar;
            Op op;
            if (!(lineStream >> val)) {
                // failed to read an integer, line was instead 'old * old'
                op = SQUARE;
                val = 0;
            }
            else {
                op = opChar == '+' ? ADD : MULT;
            }


            // Fourth line: '  Test: divisible by 2'
            lineStream = getUnindentedLine(input);
            lineStream.ignore(strlen("Test: divisible by "));
            int divTest;
            lineStream >> divTest;
            
            // Fifth line: '    If true: throw to monkey 7'
            lineStream = getUnindentedLine(input);
            lineStream.ignore(strlen("If true: throw to monkey "));
            int trueMonkey;
            lineStream >> trueMonkey;

            // Sixth line: '    If false: throw to monkey 1'
            lineStream = getUnindentedLine(input);
            lineStream.ignore(strlen("If false: throw to monkey "));
            int falseMonkey;
            lineStream >> falseMonkey;

            monkeys.emplace_back(Monkey(op, val, divTest, trueMonkey, falseMonkey, items, monkeys));

            getline(input, line); // trailing blank line
        }

        for (int i = 0; i < 20; i++) {
            for (Monkey& monkey : monkeys) {
                monkey.inspect();
            }
        }

        // Need to take a copy (of pointers), since ordering of monkeys matters for how they pass elements,
        // and can only move around pointers due to const members of Monkey preventing assignment
        vector<Monkey*> topMonkeys;
        for (Monkey& monkey : monkeys) {
            topMonkeys.emplace_back(&monkey);
        }

        std::partial_sort(topMonkeys.begin(), topMonkeys.begin() + 2, topMonkeys.end(), [](Monkey* const x, Monkey* const y) {
            return x->getInspected() > y->getInspected();
        });

        cout << topMonkeys[0]->getInspected() * topMonkeys[1]->getInspected() << endl; // 90882
    }

    int main() {
        part1();

        return 0;
    }

}