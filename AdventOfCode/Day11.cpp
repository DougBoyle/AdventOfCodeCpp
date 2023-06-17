#include <iostream>
#include <fstream>
#include <string> // getline
#include <sstream> // string -> stream
#include <queue>
#include <vector>
#include <cassert>
#include <numeric>

using namespace std;

enum Op { ADD, MULT, SQUARE };

class Monkey {
    const Op op;
    const int value;
    const int divTest;
    const int caseTrueMonkey;
    const int caseFalseMonkey;

    uint64_t inspected = 0;
    queue<int> items;
    vector<Monkey>& monkeys;
    uint64_t& lcm;

private:
    void push(uint64_t val) {
        if (val > std::numeric_limits<int>::max()) {
            throw std::runtime_error("Value " + to_string(val) + " out of range for int");
        }

        items.push(static_cast<int>(val));
    }

    uint64_t newVal(uint64_t val) {
        if (op == ADD) {
            return (val + value);
        }
        else if (op == MULT) {
            return (val * value);
        }
        else if (op == SQUARE) {
            return (val * val);
        }
        else {
            string errorMsg = "Unknown operation " + op;
            throw invalid_argument(errorMsg);
        }
    }

    void forward(uint64_t val) {
        if (val % divTest == 0) {
            monkeys[caseTrueMonkey].push(val);
        }
        else {
            monkeys[caseFalseMonkey].push(val);
        }
    }

public:
    Monkey(
        const Op op,
        const int value,
        const int divTest,
        const int caseTrueMonkey,
        const int caseFalseMonkey,
        queue<int>& items,
        vector<Monkey>& monkeys,
        uint64_t& lcm
    ) : op(op), value(value), divTest(divTest), caseTrueMonkey(caseTrueMonkey), caseFalseMonkey(caseFalseMonkey), items(items),
        monkeys(monkeys), lcm(lcm) {}


    void inspect(bool divide) {
        while (!items.empty()) {
            inspected++;
            uint64_t val = items.front(); items.pop();
            uint64_t updated = newVal(val);
            if (divide) updated /= 3;
            // can't make use of modulo when dividing by 3 in first part
            else updated %= lcm;
            forward(updated);
        }
    }

    uint64_t getInspected() {
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

// LCM needs passing in, since it goes into the Monkey objects as a reference,
// and a reference to a variable in this method will be invalid once this returns.
vector<Monkey> readMonkeys(uint64_t &lcm) {
    ifstream input("Day11.txt");

    vector<Monkey> monkeys;

    string line;
    string placeholderWord;
    int monkeyIdx;

    lcm = 1;

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
        lcm = std::lcm(lcm, divTest);

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

        monkeys.emplace_back(Monkey(op, val, divTest, trueMonkey, falseMonkey, items, monkeys, lcm));

        getline(input, line); // trailing blank line
    }

    return monkeys;
}

namespace day11 {
    uint64_t getMonkeyBusiness(vector<Monkey>& monkeys) {
        // Need to take a copy (of pointers), since ordering of monkeys matters for how they pass elements,
        // and can only move around pointers due to const members of Monkey preventing assignment
        vector<Monkey*> topMonkeys;
        for (Monkey& monkey : monkeys) {
            topMonkeys.emplace_back(&monkey);
        }

        std::partial_sort(topMonkeys.begin(), topMonkeys.begin() + 2, topMonkeys.end(), [](Monkey* const x, Monkey* const y) {
            return x->getInspected() > y->getInspected();
        });

        return topMonkeys[0]->getInspected() * topMonkeys[1]->getInspected();
    }

    void part1() {
        // unused for pt. 1
        uint64_t lcm = 1;
        vector<Monkey> monkeys = readMonkeys(lcm);

        for (int i = 0; i < 20; i++) {
            for (Monkey& monkey : monkeys) {
                monkey.inspect(true);
            }
        }

        cout << getMonkeyBusiness(monkeys) << endl; // 90882
    }

    void part2() {
        uint64_t lcm = 1;
        vector<Monkey> monkeys = readMonkeys(lcm);

        for (int i = 0; i < 10000; i++) {
            for (Monkey& monkey : monkeys) {
                monkey.inspect(false);
            }
        }

        cout << getMonkeyBusiness(monkeys) << endl; // 30893109657
    }

    int main() {
        part2();

        return 0;
    }

}