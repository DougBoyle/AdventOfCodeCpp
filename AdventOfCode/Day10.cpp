#include <iostream>
#include <fstream>
#include <string> // getline
#include <set>
#include <vector>
#include <cassert>

using namespace std;

template <typename Callback>
class Machine {
    int cycle = 0;
    int regX = 1;
    Callback callback;

public:
    Machine(const Callback& callback) : callback{ callback } {
        callback(++cycle, regX);
    }

    void addx(int n) {
        // addx on cycle n doesn't affect register until during cycle n+2
        callback(++cycle, regX);
        regX += n;
        callback(++cycle, regX);
    }

    void noop() {
        callback(++cycle, regX);
    }
};

namespace day10 {
    template<typename Callback>
    void runMachine(const Callback& callback) {
        ifstream input("Day10.txt");

        Machine machine(callback);

        string op;
        int arg;

        while (input >> op) {
            if (op == "noop") {
                machine.noop();
            }
            else if (op == "addx") {
                input >> arg;
                machine.addx(arg);
            }
            else {
                string errorMsg = "Unknown operation " + op;
                throw invalid_argument(errorMsg);
            }
        }
    }

    void part1() {
        int strength = 0;

        runMachine([&strength](const int cycle, const int regX) {
            if (cycle == 20 || cycle == 60 || cycle == 100 || cycle == 140 || cycle == 180 || cycle == 220) {
                strength += cycle * regX;
            }
        });

        cout << strength << endl; // 13520
    }

    void part2() {
        char display[6][40];

        runMachine([&display](const int cycle, const int regX)
        {
            int cycleZeroIndex = cycle - 1;
            int x = cycleZeroIndex % 40;
            int y = cycleZeroIndex / 40;

            if (y >= 6) return; // unnecessary code after display fully drawn (possibly due to proactively sending next value)

            if (x >= regX - 1 && x <= regX + 1) {
                display[y][x] = '#';
            }
            else {
                display[y][x] = '.';
            }
        });

        for (int i = 0; i < 6; i++) {
            cout << string(display[i], 40) << endl;
        }
    }


    int main() {
        part2();

        return 0;
    }

}