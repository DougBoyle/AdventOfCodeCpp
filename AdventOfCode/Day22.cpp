#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <set>
#include <algorithm>
#include <cassert>
#include <stdexcept>

#include "Point.h"
#include "Point3.h"

using namespace std;


// TODO: Possibly 1 big refactor to just actually define all edges along [0-3]^3, rather than [0-4]? (i.e. all voxels).
//       Rotations should work the same, and doesn't affect mapping of net to/from 3D, given other data present.
namespace day22 {
	enum Direction {
		RIGHT = 0,
		DOWN = 1,
		LEFT = 2,
		UP = 3,
	};

	Point<int> move(const Point<int>& p, Direction d) {
		Point<int> result = p;
		switch (d) {
		case RIGHT:
			result.x++;
			break;
		case DOWN:
			result.y++; // due to numbering of rows
			break;
		case LEFT:
			result.x--;
			break;
		case UP:
			result.y--;
			break;
		}
		return result;
	}

	const int TILE_SIZE = 50;
	
	// 0 <= original.x, y < TILE_SIZE
	Point3 mapRelativeFacePointToCube(const Point<int>& original, const Face& netFace, const Face& cubeFace, const Line& edge) {
		// work out coordinates in terms of face directions, then apply those to mapped face
		assert(netFace.numPoints() == 4);
		assert(cubeFace.numPoints() == 4);

		const Point3& originalOrigin = netFace[0];
		const Point3 originalX = (netFace[1] - originalOrigin).normalise();
		const Point3 originalY = (netFace[3] - originalOrigin).normalise();

		const Point3 original3 = original;
		int x = original3.dot(originalX);
		int y = original3.dot(originalY);

		const Point3& mappedOrigin = cubeFace[0];
		const Point3 mappedX = (cubeFace[1] - mappedOrigin).normalise();
		const Point3 mappedY = (cubeFace[3] - mappedOrigin).normalise();

		// For cube with edges [0,4]^3, actual 3D cells are [0-3]^3.
		// Returned point should be along the edge i.e. [0-4]^3, 
		// but to normalise for the two faces possibly counting along edge in reverse directions,
		// always give coordinate ALONG the edge in [0-3] range
		// One of 'edge . x' or 'edge . y' should be non-zero, reversed iteration identified by that vector having a negative component,
		// in which case need to take an extra step in that negative direction.
		// (Relying on exactly one of x/y/z components being non-zero)
		if (edge.asVector().dot(mappedX) != 0) {
			if (mappedX.x + mappedX.y + mappedX.z < 0) x++;
		}
		else {
			if (mappedY.x + mappedY.y + mappedY.z < 0) y++;
		}
		Point3 newPoint = mappedOrigin + x * mappedX + y * mappedY;

		return newPoint;
	}

	Point<int> mapCubePointToCoordsInFace(const Point3& original, const Face& cubeFace, const Face& netFace, const Line& edge) {
		assert(cubeFace.numPoints() == 4);
		assert(netFace.numPoints() == 4);

		const Point3& originalOrigin = cubeFace[0];
		const Point3 originalX = (cubeFace[1] - originalOrigin).normalise();
		const Point3 originalY = (cubeFace[3] - originalOrigin).normalise();


		const Point3 offsetOriginal = original - originalOrigin;
		int x = offsetOriginal.dot(originalX);
		int y = offsetOriginal.dot(originalY);

		// See above, axis along edge always given in range [0-3], since the two faces could iterate it in opposite directions.
		// If cube does indeed iterate it in reverse direction, we'll have calculated 1 too high an offset, and need to reduce that.
		if (edge.asVector().dot(originalX) != 0) {
			if (originalX.x + originalX.y + originalX.z < 0) x--;
		}
		else {
			if (originalY.x + originalY.y + originalY.z < 0) y--;
		}

		const Point3& mappedOrigin = netFace[0];
		const Point3 mappedX = (netFace[1] - mappedOrigin).normalise();
		const Point3 mappedY = (netFace[3] - mappedOrigin).normalise();
		Point3 newRelativePoint = x * mappedX + y * mappedY;

		/*
			Grid is (0-3, 0-3), but edges are defined along [0,4]-[0,4] to line up between cubes.
			If adjacent faces have opposite traversal directions along axis, end up being off-by-one
			when crossing onto a new face from the right or bottom (x=4 or y=4). Have to adjust accordingly.
		*/
		// Make use of points being ordered from top-left anti-clockwise, and edge following point order.
		// Hence edge starting at face[1] = bottom, and at face[2] = right
		if (cubeFace[1] == edge.from) newRelativePoint.y--;
		else if (cubeFace[2] == edge.from) newRelativePoint.x--;

		assert(newRelativePoint.z == 0);
		return { newRelativePoint.x, newRelativePoint.y };
	}

	// Again rely on net faces being defined in following order:
	//
	// (0,0) 0th      (1, 0) 3rd
	//
	// (0, 1) 1st     (1, 1) 2nd
	Direction mapCubeDirToDirInNet(const Point3 cubeDir, const Face& cubeFace) {
		assert(cubeFace.numPoints() == 4);

		const Point3& faceOrigin = cubeFace[0];
		const Point3 faceX = (cubeFace[3] - faceOrigin).normalise();
		const Point3 faceY = (cubeFace[1] - faceOrigin).normalise();

		int x = cubeDir.dot(faceX);
		int y = cubeDir.dot(faceY);

		assert(x == 0 && (y == 1 || y == -1) || y == 0 && (x == 1 || x == -1));
		return x == 1 ? RIGHT
			: x == -1 ? LEFT
			: y == 1 ? DOWN
			: UP;
	}

	Face getSharedFace(const Face& f, const Line& edge, const map<Face, Face>& cubeToNetFaces) {
		const Face* newCubeFace = nullptr;
		for (const auto& [cubeFace, _] : cubeToNetFaces) {
			if (f.touches(cubeFace) == edge) {
				return cubeFace;
			}
		}
		throw invalid_argument("No touching face found");
	}

	/*
	Makes use of the fact that any row or column has a single tiled region.
	i.e. no C like shapes:
	###
	#
	###

	Hence can track xmin/max for each row, and ymin/max for each column.
	*/
	class Grid {
	public:
		// false = empty tile, true = wall, absent = off of map
		map<Point<int>, bool> walls {};
		map<int, int> rowXMin;
		map<int, int> rowXMax;
		map<int, int> colYMin;
		map<int, int> colYMax;

		void setTile(int x, int y, bool wall) {
			walls[{x, y}] = wall;

			// careful to avoid inserting default 0 values
			if (rowXMin.contains(y)) rowXMin[y] = min(rowXMin[y], x);
			else rowXMin[y] = x;

			if (rowXMax.contains(y)) rowXMax[y] = max(rowXMax[y], x);
			else rowXMax[y] = x;

			if (colYMin.contains(x)) colYMin[x] = min(colYMin[x], y);
			else colYMin[x] = y;

			if (colYMax.contains(x)) colYMax[x] = max(colYMax[x], y);
			else colYMax[x] = y;
		}

		Point<int> move(const Point<int>& p, const Direction d, int steps) {
			Point<int> result = p;
			while (steps--) {
				Point<int> next = day22::move(result, d);
				if (walls.contains(next)) {
					if (walls.at(next)) return result;
					else result = next;
				}
				else {
					// wrap around
					switch (d) {
					case RIGHT:
						next.x = rowXMin[p.y];
						break;

					case DOWN:
						next.y = colYMin[p.x];
						break;
					case LEFT:
						next.x = rowXMax[p.y];
						break;
					case UP:
						next.y = colYMax[p.x];
						break;
					}
					assert(walls.contains(next));
					if (walls[next]) return result;
					else result = next;
				}
			}
			return result;
		}


		tuple<Point<int>, Direction, Face> getNewCoords(
			const Point<int> pointOnBoundary, const Face& cubeFace, const Face& flatFace,
			const Line& cubeEdgeCrossed, const map<Face, Face>& cubeToFlatFace)
		{
			const Point3 cubePoint = mapRelativeFacePointToCube(pointOnBoundary, flatFace, cubeFace, cubeEdgeCrossed);
			
			const Face newCubeFace = getSharedFace(cubeFace, cubeEdgeCrossed, cubeToFlatFace);
			const auto& newFlatFace = cubeToFlatFace.at(newCubeFace);

			Point<int> newFacePoint = mapCubePointToCoordsInFace(cubePoint, newCubeFace, newFlatFace, cubeEdgeCrossed.reversed());

			// Get direction in new face
			const Point3 dirOnTouchingFace = cubeFace.dirOnTouchingFace(newCubeFace);
			const Direction newDirection = mapCubeDirToDirInNet(dirOnTouchingFace, newCubeFace);
			return { newFacePoint, newDirection, newCubeFace };
		}

		tuple<Point<int>, Direction, Face> getNewPointIfNoWall(
			const Point<int>& originalPoint, const Point<int>& newPoint,
			const Direction& originalDir, const Direction& newDir,
			const Face& originalFace, const Face& newFace,
			map<Face, Face>& cubeToFlatFace)
		{
			const auto& newFlatFace = cubeToFlatFace.at(newFace);
			Point3 netPoint3 = newFlatFace[0] + newPoint;
			assert(netPoint3.z == 0);
			Point<int> netPoint = { netPoint3.x, netPoint3.y };
			assert(walls.contains(netPoint));
			if (walls.at(netPoint)) return { originalPoint, originalDir, originalFace };
			else return { newPoint, newDir, newFace };
		}
		
		// Due to how vertices identified, also need to say which face we're on while moving
		// e.g. (0,0,0) could be on any of 3 faces.
		// Both current Face and Direction on a face can change while moving.
		// args: 'p' and 'd' are as if looking at flat map (coords on flat face), face is the actual face on the cube
		tuple<Point<int>, Direction, Face> moveOnCube(const Point<int>& p, const Direction d, const Face& face, map<Face, Face>& cubeToFlatFace) {
			const Point<int>& next = day22::move(p, d);
			const auto& flatFace = cubeToFlatFace.at(face);


			// check bounds
			if (next.x < 0) {
				// Gone off 'left' of this face, need to map to/from cube to get new face
				
				// Get the edge we are about to go off of
				// Rely on point[0] of net faces being top left, so going off x=0 => left side => first edge
				// TODO: Should just define a getEdge(direction) on faces
				const Line cubeEdgeCrossed = { face[0], face[1] };

				// use original point, which is along edge (x=0)
				const auto& [newP, newDir, newFace] = getNewCoords(p, face, flatFace, cubeEdgeCrossed, cubeToFlatFace);
				return getNewPointIfNoWall(p, newP, d, newDir, face, newFace, cubeToFlatFace);
			}
			else if (next.x >= TILE_SIZE) {
				// Gone off 'right' of this face, need to map to/from cube to get new face

				// Get the edge we are about to go off of
				// Rely on point[0] of net faces being top left, so going off x=50 => right side => third edge
				const Line cubeEdgeCrossed = { face[2], face[3] };

				// use new point, which is along edge (x=50)
				const auto& [newP, newDir, newFace] = getNewCoords(next, face, flatFace, cubeEdgeCrossed, cubeToFlatFace);
				return getNewPointIfNoWall(p, newP, d, newDir, face, newFace, cubeToFlatFace);
			}
			else if (next.y < 0) {
				// Gone off 'left' of this face, need to map to/from cube to get new face

				// Get the edge we are about to go off of
				// Rely on point[0] of net faces being top left, so going off x=0 => left side => first edge
				const Line cubeEdgeCrossed = { face[3], face[0] };

				// use original point, which is along edge (x=0)
				const auto& [newP, newDir, newFace] = getNewCoords(p, face, flatFace, cubeEdgeCrossed, cubeToFlatFace);
				return getNewPointIfNoWall(p, newP, d, newDir, face, newFace, cubeToFlatFace);
			}
			else if (next.y >= TILE_SIZE) {
				// Gone off 'bottom' of this face, need to map to/from cube to get new face

				// Get the edge we are about to go off of
				// Rely on point[0] of net faces being top left, so going off y=50 => bottom side => second edge
				const Line cubeEdgeCrossed = { face[1], face[2] };

				// use new point, which is along edge (y=50)
				const auto& [newP, newDir, newFace] = getNewCoords(next, face, flatFace, cubeEdgeCrossed, cubeToFlatFace);
				return getNewPointIfNoWall(p, newP, d, newDir, face, newFace, cubeToFlatFace);
			}
			else {
				// simple case, just check if that tile is free
				return getNewPointIfNoWall(p, next, d, d, face, face, cubeToFlatFace);
			}
		}

		tuple<Point<int>, Direction, Face> moveOnCube(const Point<int>& p, const Direction d, const Face& face, map<Face, Face>& cubeToFlatFace, int steps) {
			tuple<Point<int>, Direction, Face> result = { p, d, face };
			while (steps--) {
				auto& [curP, curD, curF] = result;
				tuple<Point<int>, Direction, Face> next = moveOnCube(curP, curD, curF, cubeToFlatFace);
				if (next == result) return result; // hit wall, return early
				else result = next;
			}
			return result;
		}
	};

	Grid readGrid(ifstream& input) {
		Grid grid;

		int y = 0;
		string line;
		while (getline(input, line) && line.length() > 0) {
			y++;
			int x = 0;
			for (char c : line) {
				x++;
				switch (c) {
				case ' ': continue;
				case '.': {
					grid.setTile(x, y, false);
					break;
				}
				case '#': {
					grid.setTile(x, y, true);
					break;
				}
				default: throw invalid_argument("Unknown tile: " + c);
				}
			}
		}

		return grid;
	}

	void part1() {
		ifstream input{ "Day22.txt" };
		if (!input) throw invalid_argument("Failed to open Day22.txt");

		Grid grid = readGrid(input);

		string commands;
		getline(input, commands);
		istringstream commandStream{ commands };

		int y = 1;
		int x = grid.rowXMin[y];
		Point p{ x, y };
		// first available tile on top row
		while (grid.walls.at(p)) p.x++;

		Direction dir = RIGHT;
		char next;
		while ((next = commandStream.peek()) != EOF) {
			if (next == 'L') {
				dir = static_cast<Direction>((dir + 3) % 4);
				commandStream.get();
				continue;
			}
			else if (next == 'R') {
				dir = static_cast<Direction>((dir + 1) % 4);
				commandStream.get();
				continue;
			}
			else {
				int steps;
				commandStream >> steps;
				p = grid.move(p, dir, steps);
			}
		}

		// 1000 times the row, 4 times the column, and the facing.
		int result = 1000 * p.y + 4 * p.x + static_cast<int>(dir);

		cout << result << endl; // 109094
	}

	void checkIsCube(const vector<Face>& faces) {
		for (const auto& f1 : faces) {
			int touching = 0;
			for (const auto& f2 : faces) {
				if (f1 == f2) continue;
				if (f1.touches(f2) != Line::NONE) touching++;
			}
			if (touching != 4) throw invalid_argument("Appears to not be a cube, expected each face to have 4 neighbours at end");
		}
		cout << "Faces form a cube:" << endl;
		for (auto const& f : faces) cout << "  " << f << endl;
	}

	void treeFromNet(const vector<Face>& net, const Face& face, const Face* lastFace, map<Face, set<Face>>& dependents, set<Face>& visited) {
		assert(!visited.contains(face)); // guard against loops, and ensure whole net gets explored
		visited.insert(face);

		for (const Face& f2 : net) {
			if (face == f2 || (lastFace != nullptr && *lastFace == f2)) continue; // same face, or parent we just came from

			Line match = face.touches(f2);
			if (match == Line::NONE) continue; // not connected

			dependents[face].insert(f2);
			treeFromNet(net, f2, &face, dependents, visited);
		}
	}

	void rotateFaceAndDependents(const Face& netFace, const Line& edge, const map<Face, set<Face>>& netDependents, map<Face, Face>& netToCubeFace) {
		Face& cubeFace = netToCubeFace.at(netFace);
		cubeFace = cubeFace.turnAround(edge);

		if (!netDependents.contains(netFace)) return; // leaf node, done.
		for (const auto& dependent : netDependents.at(netFace)) {
			rotateFaceAndDependents(dependent, edge, netDependents, netToCubeFace);
		}
	}

	void rotateAndBuildCube(const Face& currentFace, const map<Face, set<Face>>& netDependents, map<Face, Face>& netToCubeFace) {
		if (!netDependents.contains(currentFace)) return; // leaf node, done.
		const Face& cubeFace = netToCubeFace.at(currentFace);

		for (const auto& f2 : netDependents.at(currentFace)) {
			Face& cubeFace2 = netToCubeFace.at(f2);

			Line edge = cubeFace.touches(cubeFace2);
			if (edge == Line::NONE) throw invalid_argument("Connected faces no longer touching?");

			rotateFaceAndDependents(f2, edge, netDependents, netToCubeFace);

			rotateAndBuildCube(f2, netDependents, netToCubeFace);
		}
	}

	// For part 2, need to be able to fold into a cube!
	// First solve the folding, then define mappings for going off one edge -> position and direction on another edge
	/*
		- Each face has points defined in counter-clockwise order looking down on the grid (remembering top is y=0).
		- Will form cube such that original faces are the INSIDE of the cube (shouldn't make a difference, but just in case).
		- Counter-clockwise order -> normal pointing inside cube, should always fold faces so far edge is in +ve direction.

		Returns the faces in the same order, but their positions after being folded into a cube
	*/
	vector<Face> buildCube(const vector<Face>& originalNet) {
		// map original -> new faces
		map<Face, Face> faceMapping;
		for (const Face& f1 : originalNet) faceMapping.insert({ f1, f1 }); // initiall unmodified 


		if (originalNet.size() != 6) throw invalid_argument("Net must have 6 faces for a cube");


		// ordering of faces to visit
		const Face& rootFace = originalNet[0];

		map<Face, set<Face>> dependents;
		set<Face> visited;
		treeFromNet(originalNet, rootFace, nullptr, dependents, visited);
		assert(visited.size() == 6); // reached everywhere on net

		// Now rotate to build cube, based on found tree
		rotateAndBuildCube(rootFace, dependents, faceMapping);

		vector<Face> result;
		for (const auto& face : originalNet) {
			result.push_back(faceMapping.at(face));
		}

		checkIsCube(result);

		return result;
	}

	void part2() {
		ifstream input{ "Day22.txt" };
		if (!input) throw invalid_argument("Failed to open Day22.txt");

		Grid grid = readGrid(input);

		// Now turn this into a collection of faces to pass
		// Exploits the fact that the net is aligned with the input shape i.e. on multiples of 50
		vector<Face> netFaces;
		for (const auto& [p, _] : grid.walls) {
			// wasteful to iterate the whole things, but straightforward
			// (could instead remember overall min/max bounds of grid, and search with 50x50 spacing)
			// NOTE: 1 INDEXED
			if (p.x % TILE_SIZE == 1 && p.y % TILE_SIZE == 1) {
				// Important that face is defined with points in anti-clockwise order!
				// GOTCHA: Need to be careful navigating 'backwards' across faces due to [0, size) ranges
				Point3 p1{ p.x, p.y, 0 };
				Point3 p2{ p.x, p.y + TILE_SIZE, 0 };
				Point3 p3{ p.x + TILE_SIZE, p.y + TILE_SIZE, 0 }; 
				Point3 p4{ p.x + TILE_SIZE, p.y, 0 };
				netFaces.push_back({ { p1, p2, p3, p4 } });
			}
		}

		if (netFaces.size() != 6) throw invalid_argument("Failed to find all 6 faces");
		cout << "Faces of net:" << endl;
		for (auto const& f : netFaces) cout << "  " << f << endl;

		vector<Face> cubeFaces = buildCube(netFaces);

		// Map of cube -> net face
		map<Face, Face> cubeToNetFace;
		for (int i = 0; i < 6; i++) {
			cubeToNetFace.insert({ cubeFaces[i], netFaces[i] });
		}
		

		int y = 1;
		int x = grid.rowXMin[y];
		Point<int> p{ x, y };
		// first available tile on top row
		while (grid.walls.at(p)) p.x++;

		// Find corresponding face on net of starting point, and which face of the cube that is
		const pair<const Face, Face>* start = nullptr;
		for (auto& cubeFaceNetFace : cubeToNetFace) {
			const auto& [cubeFace, netFace] = cubeFaceNetFace;
			int xCoord = p.x - netFace[0].x;
			int yCoord = y - netFace[0].y;
			if (xCoord >= 0 && xCoord < TILE_SIZE && yCoord >= 0 && yCoord < TILE_SIZE) {
				start = &cubeFaceNetFace;
				break;
			}
		}
		assert(start != nullptr);

		Face cubeFace = start->first;
		const Face netFace = start->second;
		// adjust from grid coordinates to coordinates on tile (0 <= x,y < TILE_SIZE)
		p.x -= netFace[0].x;
		p.y -= netFace[0].y;

		string commands;
		getline(input, commands);
		istringstream commandStream{ commands };

		Direction dir = RIGHT;
		char next;
		while ((next = commandStream.peek()) != EOF) {
			if (next == 'L') {
				dir = static_cast<Direction>((dir + 3) % 4);
				commandStream.get();
				continue;
			}
			else if (next == 'R') {
				dir = static_cast<Direction>((dir + 1) % 4);
				commandStream.get();
				continue;
			}
			else {
				int steps;
				commandStream >> steps;
				tie(p, dir, cubeFace) = grid.moveOnCube(p, dir, cubeFace, cubeToNetFace, steps);
			}
		}

		// map back to actual grid coordinate
		Point3 netPosition = cubeToNetFace.at(cubeFace)[0] + p;

		// 1000 times the row, 4 times the column, and the facing.
		int result = 1000 * netPosition.y + 4 * netPosition.x + static_cast<int>(dir);

		cout << result << endl; // 53324
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