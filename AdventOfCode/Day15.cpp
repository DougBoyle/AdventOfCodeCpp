#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <stdexcept>

using namespace std;

namespace day15 {
	enum Cell {
		BEACON,
		SENSOR,
		EMPTY
	};

	bool readCoords(ifstream& input, int& sensorX, int& sensorY, int& beaconX, int& beaconY) {
		input.ignore(strlen("Sensor at x="));
		if (!(input >> sensorX)) return false;
		input.ignore(strlen(", y="));
		input >> sensorY;

		input.ignore(strlen(": closest beacon is at x="));
		input >> beaconX;
		input.ignore(strlen(", y="));
		input >> beaconY;
		input.ignore(numeric_limits<streamsize>::max(), '\n');
		return true;
	}

	void part1() {
		ifstream input{ "Day15.txt" };

		const int yRowIndex = 2000000;
		map<int, Cell> yRow;

		int sensorX, sensorY, beaconX, beaconY;
		while (readCoords(input, sensorX, sensorY, beaconX, beaconY)) {
			if (sensorY == yRowIndex) yRow[sensorX] = SENSOR;
			if (beaconY == yRowIndex) {
				yRow[beaconX] = BEACON;
			}
			int distance = abs(sensorX - beaconX) + abs(sensorY - beaconY);
			int yRowDistance = distance - abs(yRowIndex - sensorY);

			cout << "Sensor = (" << sensorX << ", " << sensorY << "), Beacon = " << beaconX << ", " << beaconY
				<< "), distance=" << distance 
				<< ", y row distance=" << yRowDistance << endl;

			if (yRowDistance < 0) continue; // out of range to be interesting
			for (int x = sensorX - yRowDistance; x <= sensorX + yRowDistance; x++) {
				if (!yRow.contains(x)) yRow[x] = EMPTY;
			}
		}

		int nonBeaconCount = 0;
		for (const auto& [x, value] : yRow) {
			nonBeaconCount += (value != BEACON);
		}

		cout << nonBeaconCount << endl; // 5147333
	}

	void bound(int& value, int min, int max, int& distance) {
		if (value > max) {
			distance -= (value - max);
			value = max;
		}
		else if (value < min) {
			distance -= (min -value);
			value = min;
		}
	}

	void part2() {
		ifstream input{ "Day15.txt" };

		// For each y coordinate, store intervals of possible positions (in 0-4M range), reduced by processing each sensor/beacon pair.
		// At end, will have only a singly entry.
		// Alternative would be to start from (0,0), see which sensor(s) rule that out,
		// then skip to the next point that sensor doesn't rule out and repeat, until 1 valid cell found.

		// Complexity: ~30 sensor/beacon pairs * 4M rows * O(pairs) scan/update to the row = 4M * 30^2 -> a bit slow, but tractable

		const int LIMIT = 4'000'000;
		// Pairs are ordered intervals of [from, to]
		vector<pair<int, int>>* grid = new vector<pair<int, int>>[LIMIT + 1];
		vector<pair<int, int>> fullRow { { 0, LIMIT } };
		fill(grid, grid + LIMIT + 1, fullRow);


		int sensorX, sensorY, beaconX, beaconY;
		while (readCoords(input, sensorX, sensorY, beaconX, beaconY)) {

			int distance = abs(sensorX - beaconX) + abs(sensorY - beaconY);

			// adjust sensor coordinates to closest point of grid (never seems to be the case in practice)
			bound(sensorX, 0, LIMIT, distance);
			bound(sensorY, 0, LIMIT, distance);

			if (distance < 0) continue; // out of range


			int yMin = max(0, sensorY - distance);
			int yMax = min(LIMIT, sensorY + distance);

			cout << "Sensor = (" << sensorX << ", " << sensorY << "), Beacon = " << beaconX << ", " << beaconY
				<< "), distance=" << distance
				<< ", y min=" << yMin << ", y max=" << yMax << endl;

			for (int y = yMin; y <= yMax; y++) {
				int xDistance = distance - abs(y - sensorY);
				int xMin = sensorX - xDistance;
				int xMax = sensorX + xDistance;

				// remove range [xMin, xMax] from the row
				vector<pair<int, int>>& row = grid[y];
				// just do linear scan, at most numPairs distinct intervals in a row
				auto it = row.begin();
				while (it != row.end()) {
					auto& [start, end] = *it;

					if (start > xMax) break; // past relevant range, stop


					if (end < xMin) {
						it++;
						continue; // too early, skip
					}

					// xMin <= end, start <= xMax, so at least some part overlaps
					if (end <= xMax) { // end of existing interval can be truncated
						if (start >= xMin) { // completely contained
							it = row.erase(it);
						}
						else { // just truncate end
							end = xMin - 1;
							it++;
						}
						continue; // end overlapped with interval, next segment may overlap too
					}
					else { // end outside covered range, so only start/middle overlap (and no point checking further)
						if (start >= xMin) { // truncate start
							start = xMax + 1;
							it++;
						}
						else { // split out middle
							int originalEnd = end;
							end = xMin - 1;
							// insert new value to split range
							it++;
							row.insert(it, { xMax + 1, originalEnd });
						}
						break;
					}
				}
			}
		}

		// find the single remaining value
		int64_t resultX = -1;
		int64_t resultY = -1;
		for (int i = 0; i <= LIMIT; i++) {
			const auto& row = grid[i];
			if (row.size() == 0) continue;
			if (row.size() > 1) {
				throw invalid_argument("Expected at most 1 remaining value, found row " + to_string(i) + " with multiple intervals: " + to_string(row.size()));
			}
			const auto& [xMin, xMax] = row[0];
			if (xMin < xMax) {
				throw invalid_argument("Expected at most 1 remaining value, found row " + to_string(i) + " with range: " + to_string(xMin) + " to " + to_string(xMax));
			}

			if (resultX != -1) {
				throw invalid_argument("Found cell (" + to_string(xMin) + "), " + to_string(i) + ", but already had cell (" + to_string(resultX) + "," + to_string(resultY) + ")");
			}

			resultX = xMin;
			resultY = i;
		}

		cout << resultX << ", " << resultY << endl; 
		cout << 4000000 * resultX + resultY << endl; // 13734006908372

		delete[] grid;
	}

	int main() {
		part2();

		return 0;
	}
}