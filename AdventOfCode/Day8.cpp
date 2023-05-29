#include <iostream>
#include <fstream>
#include <string> // getline
#include <vector>
#include <map>

using namespace std;

enum Direction { Up = 0, Down = 1, Left = 2, Right = 3 };

Direction opposite(const Direction& d) {
    switch (d)
    {
    case Up:
        return Down;
    case Down:
        return Up;
    case Left:
        return Right;
    case Right:
        return Left;
    }
}

class Point {
public:
    int x;
    int y;

    // delegating constructor
    Point() : Point(0, 0) {}
    Point(int x, int y) : x(x), y(y) {}

    bool inBounds(int width, int height) {
        return x >= 0 && x < width&& y >= 0 && y < height;
    }

    void move(const Direction& d, int steps) {
        switch (d)
        {
        case Up:
            y += steps;
            break;
        case Down:
            y -= steps;
            break;
        case Left:
            x -= steps;
            break;
        case Right:
            x += steps;
            break;
        }
    }

    void move(const Direction& d) {
        move(d, 1);
    }

    // For scanning whole grid (ignoring edges) in direction Right,
    // start with (0, 1), and scan the row below after, until you reach (n-1, 0)
    static Point firstScanFrom(const Direction& d, int gridWidth, int gridHeight) {
        switch (d)
        {
        case Up: // start bottom left
            return Point(1, 0);
        case Down: // start top left
            return Point(1, gridHeight - 1);
        case Left:
            return Point(gridWidth - 1, 1);
        case Right:
            return Point(0, 1);
        }
    }

    static Point lastScanFrom(const Direction& d, int gridWidth, int gridHeight) {
        switch (d)
        {
        case Up: // last row is bottom right
            return Point(gridWidth - 1, 0);
        case Down: // last row is top right
            return Point(gridWidth - 1, gridHeight - 1);
        case Left:
            return Point(gridWidth - 1, gridHeight - 1);
        case Right:
            return Point(0, gridHeight - 1);
        }
    }

    void nextScanFrom(const Direction& d) {
        switch (d)
        {
        case Up: // move right across rows
        case Down:
            move(Right);
            break;
        case Left: // move up rows
        case Right:
            return move(Up);
        }
    }

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};


namespace day8 {
    // height not known in advance
    vector<vector<int>> readGrid() {
        ifstream input("Day8.txt");

        vector<vector<int>> result;


        string line;
        while (getline(input, line)) {
            vector<int> row;
            for (char c : line) {
                row.push_back(c - '0');
            }
            result.push_back(move(row));
        }

        return result;
    }

    void part1() {
        vector<vector<int>> grid = readGrid();
        int gridHeight = grid.size();
        int gridWidth = grid[0].size();
        // [m][n] only supported if n is a compile-time constant
        // {} intializes everything to 0 (or could specify prefix of array values to populate)
        bool* visible = new bool[gridHeight * gridWidth] {};


        // TODO: Work out visible cells, probably using a separate structure?
        // corners always visible, and not useful to examine,
        // whereas each edge is considered when iterating from that side to the opposite edge
        int visbleCount = 4;

        Direction dirs[] = { Up, Down, Left, Right };
        
        for (Direction d : dirs) {
            Point start = Point::firstScanFrom(d, gridWidth, gridHeight);
            Point end = Point::lastScanFrom(d, gridWidth, gridHeight);

            while (start != end) {
                Point p = start;
                int highest = -1;

                // Slightly unnecessary checking the furthest edge,
                // could have a separate 'inBoundsWithBorder(direction, width, height)' if we really wanted to avoid it
                while (p.inBounds(gridWidth, gridHeight)) {
                    int height = grid[p.y][p.x];
                    if (height > highest) {
                        if (!visible[p.y * gridWidth + p.x]) {
                            visible[p.y * gridWidth + p.x] = true;
                            visbleCount++;
                        }
                        highest = height;
                    }
                    p.move(d);
                }

                start.nextScanFrom(d);
            }
        }

        cout << visbleCount << endl; // 1708
    }

    void part2() {
        vector<vector<int>> grid = readGrid();
        int gridHeight = grid.size();
        int gridWidth = grid[0].size();

        int* visibleRange = new int[4 * gridHeight * gridWidth] {}; // all initialised to 0

        Direction dirs[] = { Up, Down, Left, Right };

        
        for (Direction d : dirs) {
            Direction back = opposite(d);
            Point start = Point::firstScanFrom(d, gridWidth, gridHeight);
            // No need to look at edge rows, will always have a multiplier of 0
            Point end = Point::lastScanFrom(d, gridWidth, gridHeight);

            while (start != end) {
                Point p = start;

                // index -> index of first tree of greater height else 0
                // from that, can jump over trees in increasing height until first tree of equal or greater height found
                vector<int> firstHigherTree;

                // Slightly unnecessary checking the furthest edge,
                // could have a separate 'inBoundsWithBorder(direction, width, height)' if we really wanted to avoid it
                int idx = 0;
                while (p.inBounds(gridWidth, gridHeight)) {
                    int height = grid[p.y][p.x];

                    int higherIdxOrEdge = idx == 0 ? 0 : idx - 1;
                    Point higherTreePos = Point(p);
                    higherTreePos.move(back); // will be ignored if we're already at the edge
                    // search until we find the edge, or a tree of equal or greater height
                    while (higherIdxOrEdge > 0 && grid[higherTreePos.y][higherTreePos.x] < height) {
                        // need to keep searching
                        int newHigherIndex = firstHigherTree[higherIdxOrEdge];
                        higherTreePos.move(back, higherIdxOrEdge - newHigherIndex);
                        higherIdxOrEdge = newHigherIndex;
                    }

                    if (higherIdxOrEdge > 0 && grid[higherTreePos.y][higherTreePos.x] == height) {
                        // equal height and not on edge, first higher tree is the higher tree (or edge) of the one we found
                        firstHigherTree.push_back(firstHigherTree[higherIdxOrEdge]);
                    }
                    else {
                        // we found the higher tree itself (or an edge)
                        firstHigherTree.push_back(higherIdxOrEdge);
                    }

                    visibleRange[static_cast<int>(d) * gridHeight * gridWidth + p.y * gridWidth + p.x] = idx - higherIdxOrEdge;

                    p.move(d);
                    idx++;
                }

                start.nextScanFrom(d);
            }
        }

        // find greatest product
        int max = 0;
        for (int i = 0; i < gridWidth * gridHeight; i++) {
            int prod = 1;
            for (int d = 0; d < 4; d++) {
                prod *= visibleRange[d * gridHeight * gridWidth + i];
            }
            if (prod > max) max = prod;
        }

        cout << max << endl; // 504000
    }

    int main() {
        part2();

        return 0;
    }

}