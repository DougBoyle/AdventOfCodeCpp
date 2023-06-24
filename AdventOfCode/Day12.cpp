#include <iostream>
#include <fstream>
#include <string> // getline
#include <sstream> // string -> stream
#include <stdexcept>
#include <vector>
#include <set>
#include <map>

#include <functional>

#include "Point.h"

using namespace std;

int getHeight(char c) {
    return c == 'S' ? 0 
        : c == 'E' ? 25 
        : c - 'a';
}

class Grid {
private:
    Point start;
    Point end;

    size_t grid_width;
    size_t grid_height;

    vector<vector<int>> grid;
    map<Point, int> distances;

    // std::priority_queue doesn't support remove/update, so can't 'improve' priority as shorter paths found.
    // instead just remove/reinsert into set whenever distance changes (remove first, update distance, reinsert).
    // (since set still supports ordering)
    const std::function<bool(const Point&, const Point&)> priorityQueueComparator = [this](const Point& p1, const Point& p2) {
        // Needs to compare by distance first, but then by points if not equal!
        auto res = distances.at(p1) <=> distances.at(p2);
        if (res == 0) return p1 < p2; // fall back to comparing points
        else return res < 0; // equivalent to less
    };

    typedef set<Point, decltype(priorityQueueComparator), std::allocator<Point>> Fringe;

    Fringe fringe{ priorityQueueComparator };

    int get(const Point& p) {
        return grid[p.y][p.x];
    }

    void explore(Point p, int currentHeight, int currentDistance, bool forwards) {
        int newHeight = get(p);
        if (forwards && newHeight > currentHeight + 1
            || !forwards && newHeight < currentHeight - 1) return;

        auto oldDist = distances.find(p);
        if (oldDist != distances.end() && oldDist->second <= currentDistance + 1) return;

        // never seen before (can't do fringe.contains(), since it does a lookup of distances in comparator)
        if (!distances.contains(p)) {
            distances[p] = currentDistance + 1;
            fringe.insert(p);
            return;
        }

        // update height, and reinsert into fringe
        if (fringe.contains(p)) fringe.erase(p);
        distances[p] = currentDistance + 1;
        fringe.insert(p);
    }

public:
    Grid(istream&& input) {
    
        vector<int> row;
        char c;
        while (input.get(c)) {
            if (c == '\n') {
                grid.push_back(row);
                row.clear();
            }
            else {
                if (c == 'S') {
                    start = Point(row.size(), grid.size());
                    distances[start] = 0;
                }
                else if (c == 'E') {
                    end = Point(row.size(), grid.size());
                }

                row.push_back(getHeight(c));
            }
        }
        if (row.size() > 0) grid.push_back(row);

        grid_height = grid.size();
        grid_width = grid[0].size();
    }

    bool isEnd(const Point& p) {
        return p == end;
    }

    typedef function<bool(const Point& p, const int& height)> PointPredicate;

    int dijkstra(bool forwards, PointPredicate finished) {
        Point beginning = forwards ? start : end;
        
        fringe.insert(beginning);

        while (!fringe.empty()) {
            Point p = *fringe.begin(); fringe.erase(fringe.begin());

            int height = get(p);
            int distance = distances[p];

            if (finished(p, height)) {
                fringe.clear();
                return distance;
            }

            if (p.x > 0) explore(Point{ p.x - 1, p.y }, height, distance, forwards);
            if (p.x < grid_width - 1) explore(Point{ p.x + 1, p.y }, height, distance, forwards);
            if (p.y > 0) explore(Point{ p.x, p.y - 1 }, height, distance, forwards);
            if (p.y < grid_height - 1) explore(Point{ p.x, p.y + 1 }, height, distance, forwards);
        }

        throw invalid_argument("Exhausted queue without finding end point");
    }
};

namespace day12 {
    void part1() {
        Grid grid(ifstream("Day12.txt"));

        int dist = grid.dijkstra(true, [&grid](const Point& p, const int& height) { return grid.isEnd(p); });

        cout << dist << endl; // 447
    }

    void part2() {
        Grid grid(ifstream("Day12.txt"));

        int dist = grid.dijkstra(false, [&grid](const Point& p, const int& height) { return height == 0; });

        cout << dist << endl; // 446
    }

    int main() {
        part2();

        return 0;
    }

}