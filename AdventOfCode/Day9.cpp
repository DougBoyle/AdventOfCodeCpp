#include <iostream>
#include <fstream>
#include <string> // getline
#include <set>
#include <vector>
#include <cassert>

using namespace std;

namespace day9 {

    class Point {
    public:
        int x;
        int y;

        // delegating constructor
        Point() : Point(0, 0) {}
        Point(int x, int y) : x(x), y(y) {}

        void move(const char dir) {
            switch (dir)
            {
            case 'U':
                y++;
                break;
            case 'D':
                y--;
                break;
            case 'L':
                x--;
                break;
            case 'R':
                x++;
                break;
            }
        }

        bool operator==(const Point&) const = default;
        // Will pick type as std::strong_ordering based on member fields comparators
        auto operator<=>(const Point&) const = default;
    };

    class Rope {
    private:
        vector<Point> knots;
        set<Point> tailVisited = { Point() };

        bool separated(const Point& head, const Point& tail) {
            return abs(head.x - tail.x) > 1 || abs(head.y - tail.y) > 1;
        }

        void catchUpTail(const Point& head, Point& tail) {
            if (!separated(head, tail)) return;

            if (head.x > tail.x) {
                tail.x++;
            }
            else if (head.x < tail.x) {
                tail.x--;
            }

            if (head.y > tail.y) {
                tail.y++;
            }
            else if (head.y < tail.y) {
                tail.y--;
            }
        }

    public:
        Rope(const int size) : knots{ size } {
            assert(size > 1);
        }

        void move(const char dir) {
            knots.front().move(dir);

            for (int i = 1; i < knots.size(); i++) {
                catchUpTail(knots[i - 1], knots[i]);
            }

            tailVisited.insert(knots.back());
        }

        void move(const char dir, int steps) {
            while (steps--) move(dir);
        }

        size_t getVisited() {
            return tailVisited.size();
        }
    };


    void simulateRope(int size) {
        ifstream input("Day9.txt");

        Rope rope(size);

        char dir;
        int steps;

        while (input >> dir >> steps) {
            rope.move(dir, steps);
        }

        cout << rope.getVisited() << endl;
    }

    void part1() {
        simulateRope(2); // 5513
    }

    void part2() {
        simulateRope(10);
    }

    int main() {
        part2();

        return 0;
    }

}