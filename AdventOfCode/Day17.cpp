#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <set>
#include <vector>
#include <array>
#include <numeric>
#include <functional>

#include "Point.h"

using namespace std;

namespace day17 {

	class Rock {
	private:
		Point<int64_t> lowerLeft;
		set<Point<int64_t>> offsets; // should all have 0 <= x,y due to being relative to lowerLeft

	public:
		explicit Rock(const Point<int64_t>& lowerLeft, const set<Point<int64_t>>& offsets) : lowerLeft(lowerLeft), offsets(offsets) {}

		static const array<set<Point<int64_t>>, 5> arrangements;

		int64_t ymin() const {
			return lowerLeft.y;
		}

		int64_t ymax() const {
			int64_t highest = 0;
			for (const auto& [ _, y ] : offsets) {
				if (y > highest) highest = y;
			}
			return lowerLeft.y + highest;
		}

		/*
			returns true if no collision, in which case lowerLeft updated, else returns false

			Coordinates indexed from 1 (so 0 is the floor tiles), to match height.
			Valid tiles are (xmin, xmax]
		*/
		bool move(const Point<int64_t>& direction, const int xmin, const int xmax, const set<Point<int64_t>>& occupied) {
			Point newLowerLeft = lowerLeft + direction;
			// floor is y=0, so lowest a rock can be is y=1
			if (newLowerLeft.x <= xmin || newLowerLeft.y <= 0) return false;

			for (const auto& offset : offsets) {
				const Point newPoint = newLowerLeft + offset;
				if (newPoint.x > xmax || occupied.contains(newPoint)) return false;
			}
			lowerLeft = newLowerLeft;
			return true;
		}

		void rest(set<Point<int64_t>>& occupied) {
			for (const auto& offset : offsets) {
				const Point newOccupied = lowerLeft + offset;
				occupied.insert(newOccupied);
			}
		}

		Point<int64_t> position() const {
			return lowerLeft;
		}

		void shift(Point<int64_t> shift) {
			lowerLeft = lowerLeft + shift;
		}
	};

	const array<set<Point<int64_t>>, 5> Rock::arrangements = { {
		{ {0, 0}, {1, 0}, {2, 0}, {3, 0} },
		{ {1, 0}, {0, 1}, {1, 1}, {2, 1}, {1, 2} },
		{ {0, 0}, {1, 0}, {2, 0}, {2, 0}, {2, 1}, {2, 2} },
		{ {0, 0}, {0, 1}, {0, 2}, {0, 3} },
		{ {0, 0}, {1, 0}, {0, 1}, {1, 1} },
	} };

	const set<Point<int64_t>>& nextArrangement(int& arrangementIdx) {
		if (++arrangementIdx == Rock::arrangements.size()) arrangementIdx = 0;
		return Rock::arrangements[arrangementIdx];
	}

	const Point<int64_t>& nextDirection(const vector<Point<int64_t>>& directions, int& dirIdx) {
		const Point<int64_t>& dir = directions[dirIdx++];
		if (dirIdx == directions.size()) dirIdx = 0;
		return dir;
	}

	vector<Point<int64_t>> readDirections() {
		ifstream input{ "Day17.txt" };
		vector<Point<int64_t>> directions;

		char dir;
		while (input >> dir) {
			if (dir == '<') directions.push_back(Point<int64_t>::LEFT);
			else if (dir == '>') directions.push_back(Point<int64_t>::RIGHT);
			else {
				throw invalid_argument("Unrecognised input: " + dir);
			}
		}

		return directions;
	}

	class Cave {
		typedef function<void(const Rock&)> RockCallback;

	private:
		const vector<Point<int64_t>>& directions;
		const Point<int64_t> offset{ 3, 4 }; // 2 empty spaces to left, 3 empty spaces below
		RockCallback rockPlacedCallback = [](auto& rock) {}; // called after a rock comes to rest, with rocksPlaced updated
		RockCallback stepCallback = [](auto& rock) {}; // called after every downwards step, before rock (possibly) comes to rest
		set<Point<int64_t>> occupied;
		Rock currentRock;

		/*
		Moves rock across then down, returning true if the rock moved down or false if it was blocked.
		*/
		bool step() {
			const Point<int64_t>& dir = nextDirection(directions, dirIdx);

			currentRock.move(dir, 0, 7, occupied);

			bool falling = currentRock.move(Point<int64_t>::DOWN, 0, 7, occupied);

			stepCallback(currentRock);

			if (!falling) {
				currentRock.rest(occupied);

				int64_t newYMax = currentRock.ymax();
				if (newYMax > ymax) ymax = newYMax;

				rocksPlaced++;

				rockPlacedCallback(currentRock);

				// next rock to start
				currentRock = Rock(Point<int64_t>(0, ymax) + offset, nextArrangement(arrangementIdx));
			}

			return falling;
		}

	public:
		// TODO: Make these private instead
		int arrangementIdx = 0; // currently selected rock
		int dirIdx = 0;
		int64_t rocksPlaced = 0;
		int64_t ymax = 0;

		Cave(const vector<Point<int64_t>>& directions) :
			directions(directions),
			currentRock(Rock(offset, Rock::arrangements[0])) {}

		void setStepCallback(RockCallback callback) {
			stepCallback = callback;
		}

		void setRockPlacedCallback(RockCallback callback) {
			rockPlacedCallback = callback;
		}

		void runUntil(function<bool()> p) {
			while (!p()) step();
		}

		void skip(size_t heightSkipped, size_t rocksSkipped) {
			rocksPlaced += rocksSkipped;

			set<Point<int64_t>> newOccupied;
			for (const auto& [x, y] : occupied) newOccupied.insert(Point<int64_t>(x, y + heightSkipped));
			occupied = newOccupied;

			// also need to update ymax and the 'next' rock to place
			ymax += heightSkipped;
			currentRock.shift(Point<int64_t>(0, heightSkipped));

		}
	};

	void part1() {
		vector<Point<int64_t>> directions = readDirections();
		Cave cave{ directions };

		cave.runUntil([&]() {return cave.rocksPlaced == 2022; });

		cout << "At end, ymax = " << cave.ymax << endl; // 3168
	}

	void part2() {
		vector<Point<int64_t>> directions = readDirections();
		Cave cave{ directions };

		// Cycle - direction index reaching the same value, when the same rock has just been selected as previously.
		// Also verify at that point that the 'floor' is effectively the same:
		//	Strictly - anything that went below ymax at the point the new rock had been selected, ends up in same place.
		//  Simple verification - with another cycle of rocks (or 2 after this), get exact same layout
		// Given it's a cycle, look at first 'new' rock after direction index resets
		// More generally, due to rocks taking a few steps to fall, could actually be several alternating cycles,
		// so don't overwrite the index after the first time it is set.
		vector<int> perRockCycleIdx;
		vector<int64_t> perRockRocksPlaced; // work out how large the loop is
		
		// validate loop
		int64_t loopRockCount = -1;
		int loopNextRockIdx;
		int loopNextDirIdx;

		for (const auto& arrangement : Rock::arrangements) {
			perRockCycleIdx.push_back(-1);
			perRockRocksPlaced.push_back(0);
		}
		perRockCycleIdx[0] = 0; // first action first rock is the direction at index 0
		bool firstRockAfterCycle = false;

		cave.setStepCallback([&](auto& rock) {
			if (cave.dirIdx == 0) firstRockAfterCycle = true;
		});
		cave.setRockPlacedCallback([&](auto& rock) {
			int nextRockIdx = cave.arrangementIdx;

			if (firstRockAfterCycle) {
				firstRockAfterCycle = false;
				int previousCycleIdx = perRockCycleIdx[nextRockIdx];
				if (previousCycleIdx == -1) {
					// first time we've seen this as the first rock after a loop
					perRockCycleIdx[nextRockIdx] = cave.dirIdx;
					perRockRocksPlaced[nextRockIdx] = cave.rocksPlaced;
				}
				else if (previousCycleIdx == cave.dirIdx) {
					// found a loop!
					loopNextDirIdx = cave.dirIdx;
					loopNextRockIdx = nextRockIdx;
					loopRockCount = cave.rocksPlaced - perRockRocksPlaced[nextRockIdx];
				}
			}
		});

		cave.runUntil([&]() {return loopRockCount != -1; });

		cave.setStepCallback([](auto& rock) {});

		cout << "Loop on dirIdx=" << loopNextDirIdx << " and rockIdx=" << loopNextRockIdx << ", num rocks=" << loopRockCount
			<< ", starting from rocks placed=" << cave.rocksPlaced << endl;

		// validate that we have a loop:
		// every 1700 rocks, get to next rock being rock index 2, next direction being direction index 2

		// first loop, remember where every rock placed
		int64_t ymaxBeforeLoop = cave.ymax;
		int64_t loopEnd = cave.rocksPlaced + loopRockCount;
		vector<Point<int64_t>> loopRockPositions;

		// store position of each rock
		cave.setRockPlacedCallback([&loopRockPositions](const Rock& placed) {
			loopRockPositions.push_back(placed.position());
		});

		cave.runUntil([&]() {return cave.rocksPlaced == loopEnd; });
		
		int64_t ymaxAfterLoop = cave.ymax;
		int64_t loopHeight = ymaxAfterLoop - ymaxBeforeLoop;

		cout << "Loop height = " << loopHeight << endl;

		// second loop, confirm only difference is height (based on how big one loop is)
		int64_t loopStart = cave.rocksPlaced;
		loopEnd = cave.rocksPlaced + loopRockCount;

		// validate position
		cave.setRockPlacedCallback([&](const Rock& placed) {
			if (placed.position() != loopRockPositions[cave.rocksPlaced - 1 - loopStart] + Point<int64_t>(0, loopHeight)) {
				throw invalid_argument("New rock in unexpected position, not a loop!");
			}
		});

		cave.runUntil([&]() {return cave.rocksPlaced == loopEnd; });

		cave.setRockPlacedCallback([](auto& placed) {}); // done verifying

		int64_t targetRocks = 1'000'000'000'000;
		int64_t cyclesToSkip = (targetRocks - cave.rocksPlaced) / loopRockCount;
		int64_t rocksSkipped = cyclesToSkip * loopRockCount;
		int64_t heightSkipped = cyclesToSkip * loopHeight;

		// update both accordingly
		cave.skip(heightSkipped, rocksSkipped);

		cout << "Skipped " << rocksSkipped << " rocks, now at " << cave.rocksPlaced << ", remaining=" << targetRocks - cave.rocksPlaced << endl;

		cave.runUntil([&]() {return cave.rocksPlaced == targetRocks; });

		cout << "Final ymax=" << cave.ymax << endl; // 1554117647070
	}


	void main() {
		part1();
		part2();
	}
}