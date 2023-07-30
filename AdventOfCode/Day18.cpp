#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <set>
#include <vector>
#include <numeric>
#include <functional>

using namespace std;

namespace day18 {
	typedef tuple<int, int, int> Voxel;

	class Shape {
	public:
		// bounds
		int xmin = numeric_limits<int>::max();
		int xmax = numeric_limits<int>::min();
		int ymin = numeric_limits<int>::max();
		int ymax = numeric_limits<int>::min();
		int zmin = numeric_limits<int>::max();
		int zmax = numeric_limits<int>::min();

		const set<Voxel> cells;

		Shape(const set<Voxel>& cells) : cells(cells) {
			for (const auto& [x, y, z] : cells) {
				xmin = min(x, xmin);
				xmax = max(x, xmax);
				ymin = min(y, ymin);
				ymax = max(y, ymax);
				zmin = min(z, zmin);
				zmax = max(z, zmax);
			}
		}
	};

	Shape readShape() {
		ifstream input{ "Day18.txt" };
		set<Voxel> result;

		string line;
		while (getline(input, line)) {
			istringstream iss{ line };
			int x, y, z;
			char comma;

			iss >> x >> comma >> y >> comma >> z;
			result.insert({ x, y, z });
		}

		return { result };
	}

	void forEachNeighbour(const Voxel& point, const function<void(const Voxel&)>& op) {
		const auto& [x, y, z] = point;

		op({ x - 1, y, z });
		op({ x + 1, y, z });
		op({ x, y - 1, z });
		op({ x, y + 1, z });
		op({ x, y, z - 1 });
		op({ x, y, z + 1 });
	}

	void part1() {
		Shape shape = readShape();
		const auto& cells = shape.cells;

		int openFaces = 0;
		for (const auto& point : cells) {
			forEachNeighbour(point, [&](const auto& neighbour) {
				if (!cells.contains(neighbour)) openFaces++;
			});
		}

		cout << "Open faces: " << openFaces << endl; // 3448
	}

	/* various options of different complexity:
	* 1. Draw a beam from each surface, counting intersections to determine if 'internal' or 'external'
	* 2. Build surfaces by working out which open faces are 'touching', then just see which face is obviously external
	*		e.g. leftmost face of leftmost cube.
	*		Gets complex tracking surfaces/if two faces on adjacent cubes count as joined or not (no diagnoal cube separating them)
	* 3. Bound space around the shape, add a buffer, and fill with a space-filling 'steam' shape.
	*		Then either count internal faces of trivial 'steam' object, or faces of shape facing the steam
	*/
	void part2() {
		Shape shape = readShape();
		const auto& cells = shape.cells;

		// build bounding shape with 1 cell padding
		int xmin = shape.xmin - 1;
		int xmax = shape.xmax + 1;
		int ymin = shape.ymin - 1;
		int ymax = shape.ymax + 1;
		int zmin = shape.zmin - 1;
		int zmax = shape.zmax + 1;

		// build filling shape
		set<Voxel> filledSpace{ { xmin, ymin, zmin } }; // initial cell
		vector<Voxel> queue = { {xmin, ymin, zmin} };

		while (!queue.empty()) {
			Voxel point = queue.back(); queue.pop_back();
			forEachNeighbour(point, [&](const auto& neighbour) {
				const auto& [x, y, z] = neighbour;
				if (x < xmin || x > xmax || y < ymin || y > ymax || z < zmin || z > zmax) return;

				if (cells.contains(neighbour) || filledSpace.contains(neighbour)) return;

				filledSpace.insert(neighbour);
				queue.push_back(neighbour);
			});
		}

		// having identified filled space, now check touching neighbours
		int openFaces = 0;
		for (const auto& point : cells) {
			forEachNeighbour(point, [&](const auto& neighbour) {
				if (filledSpace.contains(neighbour)) openFaces++;
			});
		}

		cout << "Open faces: " << openFaces << endl; // 2052
	}

	void main() {
		part2();
	}
}
