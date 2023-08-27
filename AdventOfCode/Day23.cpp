#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <unordered_set>
#include <deque>
#include <algorithm>
#include <stdexcept>
#include <bitset>
#include <boost/pool/pool_alloc.hpp>

#include "Point.h"

using namespace std;

namespace day23 {

	// Annoyingly, have to give all of the other defaults in order to just change the last one.
	// Using boost pool allocator, since updates will remove and re-insert elements, without actually growing/shrinking set
	typedef unordered_set<Point<int>, std::hash<Point<int>>, std::equal_to<Point<int>>, boost::fast_pool_allocator<Point<int>>> ElfGrid;

	// In order considered by elves
	enum Direction {
		UP = 0,
		DOWN = 1,
		LEFT = 2,
		RIGHT = 3,
	};

	Point<int> asVector(const Direction& d) {
		switch (d) {
		case UP: return { 0, -1 };
		case DOWN: return { 0, 1 };
		case LEFT: return { -1, 0 };
		case RIGHT: return { 1, 0 };
		default: throw invalid_argument("Unrecognised direction: " + d);
		}
	}

	void printGrid(const unordered_set<Point<int>>& elves) {
		int xMin = numeric_limits<int>::max(), yMin = numeric_limits<int>::max();
		int xMax = numeric_limits<int>::min(), yMax = numeric_limits<int>::min();

		for (const auto& [x, y] : elves) {
			xMin = min(x, xMin);
			xMax = max(x, xMax);
			yMin = min(y, yMin);
			yMax = max(y, yMax);
		}

		cout << "xMin=" << xMin << " xMax=" << xMax << " yMin=" << yMin << " yMax=" << yMax << endl;

		cout << "Occupied=" << elves.size() << endl;

		for (int y = yMin; y <= yMax; y++) {
			for (int x = xMin; x <= xMax; x++) {
				if (elves.contains({ x, y })) {
					cout << '#';
				}
				else {
					cout << '.';
				}
			}
			cout << endl;
		}
	}

	ElfGrid readGrid(ifstream& input) {
		ElfGrid elves;

		string line;
		int y = 0;
		while (getline(input, line) && line.length() > 0) {
			y++;
			int x = 0;
			for (char c : line) {
				x++;
				switch (c) {
				case '.': continue;
				case '#': {
					elves.insert({ x, y });
					break;
				}
				default: throw invalid_argument("Unknown tile: " + c);
				}
			}
		}

		return elves;
	}

	/*
	Returns whether at least 1 elf moved. Updates searchOrder at end of pass.
	*/
	bool update(ElfGrid& elves, deque<Direction>& searchOrder) {
		bool changed = false;
		vector<pair<Point<int>, Point<int>>> proposedMoves;
		unordered_map<Point<int>, int> proposedCount;

		bitset<4> occupiedSides;
		for (const auto& elf : elves) {
			// work out neighbours
			bool allEmpty = true;
			occupiedSides.reset();

			// TODO: If we reverted back to an ordered set, could simplify iteration/search to exploit the fact we know neighbours will be adjacent.
			// i.e. that entries before/after the current point are the neighbours we want, rather than full log(N) search of set.
			for (int x = -1; x <= 1; x++) {
				for (int y = -1; y <= 1; y++) {
					if (x == 0 && y == 0) continue;

					const Point<int> offset = { x, y };
					if (elves.contains(elf + offset)) {
						allEmpty = false;

						for (int d = 0; d < 4; d++) {
							const Direction dir = static_cast<Direction>(d);
							if (asVector(dir).dot(offset) == 1) occupiedSides.set(dir);
						}
					}
				}
			}

			// decide next move, if any
			if (allEmpty) continue;
			for (const Direction& dir : searchOrder) {
				if (!occupiedSides[dir]) {
					const Point<int> proposed = elf + asVector(dir);
					proposedMoves.emplace_back(elf, proposed);
					proposedCount[proposed]++;
					break;
				}
			}
		}

		// do the moves
		for (const auto& [from, to] : proposedMoves) {
			if (proposedCount.at(to) == 1) {
				elves.erase(from);
				elves.insert(to);
				changed = true;
			}
		}

		// rotate first choice
		const Direction first = searchOrder.front(); searchOrder.pop_front();
		searchOrder.push_back(first);

		return changed;
	}

	/*
	Up is y--, as top row is y=1
	*/
	void part1() {
		ifstream input{ "Day23.txt" };
		if (!input) throw invalid_argument("Failed to open Day23.txt");

		// Not particularly memory efficient, but unlikely to matter
		ElfGrid elves = readGrid(input);

		deque<Direction> searchOrder = { UP, DOWN, LEFT, RIGHT };
		for (int i = 0; i < 10; i++) {
			update(elves, searchOrder);
		}

		// find bounds of grid
		int xMin = numeric_limits<int>::max(), yMin = numeric_limits<int>::max();
		int xMax = numeric_limits<int>::min(), yMax = numeric_limits<int>::min();

		

		for (const auto& [x, y] : elves) {
			xMin = min(x, xMin);
			xMax = max(x, xMax);
			yMin = min(y, yMin);
			yMax = max(y, yMax);
		}

		cout << "xMin=" << xMin << " xMax=" << xMax << " yMin=" << yMin << " yMax=" << yMax << endl;

		// +1, since single-cell grid has xMin=xMax
		int gridArea = (xMax + 1 - xMin) * (yMax + 1 - yMin);
		int freeTiles = gridArea - static_cast<int>(elves.size());

		cout << freeTiles << endl; // 4109
	}

	void part2() {
		ifstream input{ "Day23.txt" };
		if (!input) throw invalid_argument("Failed to open Day23.txt");

		ElfGrid elves = readGrid(input);

		deque<Direction> searchOrder = { UP, DOWN, LEFT, RIGHT };
		int round = 0;
		bool changing = true;
		while (changing) {
			round++;
			changing = update(elves, searchOrder);
		}

		cout << round << endl; // 1055
	}

	/*
	Very slow to run, takes multiple minutes, need to tidy up to reduce memory usage?
	Original: 103s
	1. Already storing updates as a map of from -> to, so can update elves in-place second pass,
		and avoid iterating elves just to see which are then in the map of updates: 75s
	2. Having removed need to lookup elves in map of updates, can just store them in a list, not a sorted map: 72s
	3. Profile shows 40% of time spent on set comparison, instead use unordered_set with boost hash: 53s
	4. Now 50% creating/destroying maps. Instead clear and reuse one map for occupied directions: 23s
	5. Still ~30% modifying set of directions, switch to bitset for much lower memory footprint: 12s
	6. Also replacing map with unordered_map for tracking of conflicts: 10s
	7. pool_allocator actually worse (24s), but fast_pool_allocator (due to non-contiguous updates): 8s

	At this point, rest is just searching for cells. Could probably make faster by going back to sorted collection
	and making the different iterations related, to e.g. only search adjacent columns to see if neighbours occupied.
	*/
	void main() {
		time_t start, end;
		time(&start);
		part1();
		part2();
		time(&end);

		cout << "Elapsed: " << (double)(end - start) << endl;
	}
}