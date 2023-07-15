#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <set>
#include <algorithm>

using namespace std;

namespace day14 {

	void order(int& x, int& y, int& from, int& to) {
		if (x < y) {
			from = x;
			to = y;
		}
		else {
			from = y;
			to = x;
		}
	}

	bool readCoords(istringstream& input, int& x, int& y, int& yMax) {
		char comma;
		if (!(input >> x >> comma >> y)) return false;
		yMax = max(yMax, y);
		input.ignore(strlen(" -> ")); // no-op for last entry of line
		return true;
	}

	// represent as map of x -> set of occupied y coords
	// keeps structure relatively sparse, easy to build initially, and scan to where sand falls to in column
	// log(n) to find location in neighbour column though.
	// returns max y coordinate
	int readStructure(ifstream& input, map<int, set<int>>& occupied) {
		int yMax = 0;

		string line;
		while (getline(input, line)) {
			istringstream linestream{ line };
			int lastX, lastY;
			readCoords(linestream, lastX, lastY, yMax);

			int x, y;
			while (readCoords(linestream, x, y, yMax)) {
				int from, to;
				if (x == lastX) {
					// vertical
					set<int>& column = occupied[x]; // creates empty set if not present

					order(y, lastY, from, to);

					for (int i = from; i <= to; i++) {
						column.insert(i);
					}
				}
				else {
					// horizontal
					order(x, lastX, from, to);

					for (int i = from; i <= to; i++) {
						occupied[i].insert(y); // creates empty set first if not present
					}
				}

				lastX = x;
				lastY = y;
			}
		}

		return yMax;
	}

	void printStructure(const map<int, set<int>>& occupied, const map<int, set<int>>& original, int floor) {
		if (occupied.size() == 0) return;
		int xMin = occupied.begin()->first;
		int xMax = (occupied.rbegin())->first;

		for (int y = 0; y < floor; y++) {
			for (int x = xMin; x <= xMax; x++) {
				if (!occupied.contains(x) || !occupied.at(x).contains(y)) cout << '.';
				else if (!original.contains(x) || !original.at(x).contains(y)) cout << 'o';
				else cout << '#';
			}
			cout << endl;
		}

		// floor
		for (int x = xMin; x <= xMax; x++) cout << '#';
		cout << endl;
	}

	set<int>& getColumn(map<int, set<int>>& occupied, int x, set<int>& defaultColumn) {
		if (!occupied.contains(x)) occupied[x] = defaultColumn;
		return occupied[x];
	}

	int fill(map<int, set<int>>& occupied, set<int>& defaultColumn) {
		int sandCount = 0;
		bool progress = true;
		while (progress) {
			progress = false;
			int x = 500, y = 0;
			set<int>& column = getColumn(occupied, x, defaultColumn);
			if (column.contains(y)) break; // source blocked

			while (true) {
				set<int>& column = getColumn(occupied, x, defaultColumn);

				auto below = column.lower_bound(y);
				if (below == column.end()) break; // falls forever
				// will go to this level and 1 to the side if possible
				int belowY = *below;

				// try falling left
				set<int>& left = getColumn(occupied, x - 1, defaultColumn);
				if (!left.contains(belowY)) {
					x -= 1;
					y = belowY;
					continue;
				}

				// try falling right
				set<int>& right = getColumn(occupied, x + 1, defaultColumn);
				if (!right.contains(belowY)) {
					x += 1;
					y = belowY;
					continue;
				}

				// just insert at bottom of current column
				column.insert(belowY - 1);
				progress = true;
				sandCount++;
				break;
			}
		}

		return sandCount;
	}

	void part1() {
		ifstream input{ "Day14.txt" };

		map<int, set<int>> occupied;
		int yMax = readStructure(input, occupied);

		set<int> defaultColumn {};

		int sandCount = fill(occupied, defaultColumn);

		cout << sandCount << endl; // 774
	}

	void part2() {
		ifstream input{ "Day14.txt" };

		map<int, set<int>> occupied;
		int yMax = readStructure(input, occupied);
		int yFloor = yMax + 2;

		set<int> defaultColumn{ yFloor };

		// fill in the 'floor'
		for (auto& [key, column] : occupied) {
			column.insert(yFloor);
		}

		int sandCount = fill(occupied, defaultColumn);

		cout << sandCount << endl; // 22499
	}


	int main() {
		part2();

		return 0;
	}
}