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

#include "Point.h"

using namespace std;

namespace day24 {
	typedef Point<int> Pt;

	/*
	Up/Down flipped since grid read from top down, so y=0 is top
	*/
	const Pt& dirFromChar(const char c) {
		switch (c) {
		case '^': return Pt::DOWN;
		case '>': return Pt::RIGHT;
		case 'v': return Pt::UP;
		case '<': return Pt::LEFT;
		default: throw invalid_argument("Unrecognised direction: " + c);
		}
	}

	class Blizzard {
	public:
		Pt pos;
		Pt direction;

		Blizzard(const Pt& pos, const Pt& direction) : pos(pos), direction(direction) {}
	};

	class Player {
	public:
		bool present{ false }; // used to invalidate cells when no longer present
		int timesReachedEnd{ 0 };
		int timesReachedStart{ 0 };

		void update(const Player& other) {
			assert(other.present);
			if (!present) { // absent -> present, take actual values
				present = other.present;
				timesReachedEnd = other.timesReachedEnd;
				timesReachedStart = other.timesReachedStart;
			}
			else { // can take max, goes in sequence (0,0), (0,1), (1,1), (1,2)
				timesReachedEnd = max(timesReachedEnd, other.timesReachedEnd);
				timesReachedStart = max(timesReachedStart, other.timesReachedStart);
			}
		}
	};

	class Cell {
	private:
		bool wall;
		int blizzardCount;

	public:
		Cell(const bool wall, const int blizzardCount) : wall(wall), blizzardCount(blizzardCount) {}

		bool isWall() const {
			return wall;
		}

		int getBlizzardCount() const {
			return blizzardCount;
		}

		void incCount() {
			blizzardCount++;
		}

		void decCount() {
			blizzardCount--;
		}
	};

	class Grid {
	private:
		vector<vector<Cell>> cells;
		vector<Blizzard> blizzards;

		// Alternate between them to get next state
		vector<vector<Player>> tilesA;
		vector<vector<Player>> tilesB;

		vector<vector<Player>>* occupiedPtr{ &tilesA };
		vector<vector<Player>>* nextOccupiedPtr{ &tilesB };

		int width;
		int height;

	private:
		Cell& get(const Pt& point) {
			return cells[point.y][point.x];
		}

		bool isFree(int x, int y) {
			// need to catch y < 0 or y >= height, since no wall above entrance/below exit
			if (y < 0 || y >= height) return false;
			const Cell& cell = get({ x, y });
			return !cell.isWall() && cell.getBlizzardCount() == 0;
		}

		void updateBlizzards() {
			// exploits fact that walls are at edges x,y = 0 or x,y = width-1, height-1, and blizzards never cross through openings
			for (auto& blizzard : blizzards) {
				get(blizzard.pos).decCount();
				auto& pos = blizzard.pos;

				// wrap around walls
				pos = pos + blizzard.direction;
				if (get(pos).isWall()) {
					if (pos.x == 0) pos.x = width - 2;
					else if (pos.x == width - 1) pos.x = 1;
					else if (pos.y == 0) pos.y = height - 2;
					else if (pos.y == height - 1) pos.y = 1;
					else {
						throw invalid_argument("Unexpected wall!");
					}
				}

				get(pos).incCount();
			}
		}

		void update(const Player& from, const int newY, Player& to) {
			to.update(from);
			// going towards exit
			if (to.timesReachedEnd <= to.timesReachedStart) {
				if (newY == height - 1) to.timesReachedEnd++;
			} // going towards entrance
			else {
				if (newY == 0) to.timesReachedStart++;
			}
		}

		void updatePlayer(const int x, const int y, vector<vector<Player>>& occupied, vector<vector<Player>>& nextOccupied, int& maxTimesReachedExit) {
			Player& from = occupied[y][x];
			if (!from.present) return;

			if (isFree(x, y)) update(from, y, nextOccupied[y][x]);
			if (isFree(x - 1, y)) update(from, y, nextOccupied[y][x - 1]);
			if (isFree(x + 1, y)) update(from, y, nextOccupied[y][x + 1]);
			if (isFree(x, y - 1)) update(from, y - 1, nextOccupied[y - 1][x]);
			if (isFree(x, y + 1)) {
				Player& other = nextOccupied[y + 1][x];
				update(from, y + 1, other);
				// in case this is the first way found to reach exit n times
				maxTimesReachedExit = max(maxTimesReachedExit, other.timesReachedEnd);
			}

			from.present = false;
		}

	public:
		Grid(const vector<vector<Cell>>& cells, const vector<Blizzard>& blizzards) : cells(cells), blizzards(blizzards) {
			height = static_cast<int>(cells.size());
			width = static_cast<int>(cells[0].size());

			vector<vector<Player>>& occupied = *occupiedPtr;
			vector<vector<Player>>& nextOccupied = *nextOccupiedPtr;

			// pre-populate maps initially
			for (int y = 0; y < height; y++) {
				vector<Player> occupiedRow;
				vector<Player> nextOccupiedRow;
				for (int x = 0; x < width; x++) {
					Player p{};
					p.present = y == 0 && !get({ x, y }).isWall(); // start in top row
					occupiedRow.push_back(p);
					nextOccupiedRow.push_back({});
				}
				occupied.push_back(occupiedRow);
				nextOccupied.push_back(nextOccupiedRow);
			}
		}

		/*
		Update positions of blizzards, then of occupied cells.
		Returns max times could have reached end (going back to start in between)
		*/
		int step() {
			updateBlizzards();

			vector<vector<Player>>& occupied = *occupiedPtr;
			vector<vector<Player>>& nextOccupied = *nextOccupiedPtr;

			int maxTimesReachedExit = 0;

			// with updated blizzard positions, work out new available spaces
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					updatePlayer(x, y, occupied, nextOccupied, maxTimesReachedExit);
				}
			}

			// 'next' occupied table becomes the current one, original one now all false
			swap(occupiedPtr, nextOccupiedPtr);

			return maxTimesReachedExit;
		}
	};


	Grid readGrid(ifstream& input) {
		vector<vector<Cell>> cells;
		vector<Blizzard> blizzards;

		string line;
		while (getline(input, line) && line.length() > 0) {
			vector<Cell> row;
			for (char c : line) {
				switch (c) {
				case '#': {
					row.emplace_back(true, 0);
					continue;
				}
				case '.': {
					row.emplace_back(false, 0);
					continue;
				}
				default:
					const Pt dir = dirFromChar(c);
					Pt pos{ static_cast<int>(row.size()), static_cast<int>(cells.size()) };
					blizzards.emplace_back(pos, dir);
					row.emplace_back(false, 1);
				}
			}
			cells.push_back(row);
		}

		return {cells, blizzards};
	}

	void part1() {
		ifstream input{ "Day24.txt" };
		if (!input) throw invalid_argument("Failed to open Day24.txt");

		Grid grid = readGrid(input);

		int steps = 1;
		while (!grid.step()) steps++;

		cout << steps << endl; // 221
	}

	void part2() {
		ifstream input{ "Day24.txt" };
		if (!input) throw invalid_argument("Failed to open Day24.txt");

		Grid grid = readGrid(input);

		int steps = 1;
		while (grid.step() < 2) steps++;

		cout << steps << endl; // 739
	}

	void main() {
		time_t start, end;
		time(&start);
		part1();
		part2();
		time(&end);

		cout << "Elapsed: " << (double)(end - start) << endl;
	}
}