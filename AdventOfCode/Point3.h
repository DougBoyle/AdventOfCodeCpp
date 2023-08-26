#pragma once

#include <compare>
#include <vector>
#include <stdexcept>

class Point3 {
public:
	int x;
	int y;
	int z;

	Point3() : Point3(0, 0, 0) {}
	Point3(const int x, const int y, const int z) : x(x), y(y), z(z) {}
	Point3(const Point<int>& point2) : Point3(point2.x, point2.y, 0) {}

	bool operator==(const Point3&) const = default;

	auto operator<=>(const Point3&) const = default;

	Point3 operator+(const Point3& p) const {
		return Point3(x + p.x, y + p.y, z + p.z);
	}

	Point3 operator-(const Point3& p) const {
		return Point3(x - p.x, y - p.y, z - p.z);
	}

	Point3 normalise() const {
		// Simplified, we only care about things that are axis-aligned
		int nonZero = (x != 0) + (y != 0) + (z != 0);
		int scale = std::abs(x) + std::abs(y) + std::abs(z);
		if (nonZero != 1) throw std::invalid_argument("Only expect to normalise axis-aligned vectors");
		return { x / scale, y / scale, z / scale };
	}

	Point3 abs() const {
		return { std::abs(x), std::abs(y), std::abs(z) };
	}

	int sum() const {
		return x + y + z;
	}

	int dot(const Point3& v) const {
		return x * v.x + y * v.y + z * v.z;
	}

	Point3 operator*(int scalar) const {
		return { x * scalar, y * scalar, z * scalar };
	}

	const static Point3 UP;
	const static Point3 DOWN;
	const static Point3 LEFT;
	const static Point3 RIGHT;
	const static Point3 FORWARD;
	const static Point3 BACK;
};

const Point3 Point3::LEFT{ -1, 0, 0 };
const Point3 Point3::RIGHT{ 1, 0, 0 };
const Point3 Point3::UP{ 0, 1, 0 };
const Point3 Point3::DOWN{ 0, -1, 0 };
const Point3 Point3::FORWARD{ 0, 0, 1 };
const Point3 Point3::BACK{ 0, 0, -1 };

inline Point3 operator*(const int scalar, const Point3& v) {
	return v * scalar;
}

inline std::ostream& operator<<(std::ostream& stream, Point3 const& p) {
	return stream << "(" << p.x << ", " << p.y << ", " << p.z << ")";
}

class Line {
public:
	Point3 from;
	Point3 to;

	Line(const Point3& from, const Point3& to) : from(from), to(to) {}

	bool operator==(const Line&) const = default;
	auto operator<=>(const Line&) const = default;

	Line reversed() const {
		return { to, from };
	}

	Point3 asVector() const {
		return to - from;
	}


	const static Line NONE;
};
const Line Line::NONE({}, {}); // the line (0,0,0) to (0,0,0)

inline std::ostream& operator<<(std::ostream& stream, Line const& l) {
	return stream << "" << l.from << " -> " << l.to << ")";
}

/* Points should be defined in counter-clockwise order from (0,0), looking down on the face, where x,y looks like:
  (0, 0)     (x, 0)

  (0, y)     (x, y)
*/
class Face {
public:
	std::vector<Point3> points;

	Face(const std::vector<Point3>& points) : points(points) {}

	bool operator==(const Face&) const = default;
	auto operator<=>(const Face&) const = default;

	const Point3& operator[](const int idx) const {
		// modulo indexing allows treating points as cyclic
		return at(idx);
	}

	const Point3& at(const int idx) const {
		// modulo indexing allows treating points as cyclic
		return points[(numPoints() + idx) % numPoints()];
	}

	int numPoints() const {
		return static_cast<int>(points.size());
	}

	/*
	Returns the first shared edge found between two faces (the edge of this face, not the other).
	Relies on all faces having points defined in same order,
	so expect the shared line to be (s1[i], s1[i+1]) = (s2[j+1], s2[j])
	(accounting for possibly looping round s1[n-1] -> s1[0])
	*/
	Line touches(const Face& other) const {
		// First find matching point s1[i], then check if edge s1[i] to s1[i+1] is present
		for (int i = 0; i < numPoints(); i++) {
			const Point3& p1 = at(i);
			const Point3& p2 = at(i + 1);
			for (int j = 0; j < other.numPoints(); j++) {
				const Point3& otherP1 = other.at(j);
				if (p1 != otherP1) continue;

				const Point3& otherP2 = other.at(j - 1);
				if (p2 == otherP2) return { p1, p2 };
			}
		}
		return Line::NONE;
	}

	// Expect this and other face to share an edge.
	// if matching points on other face are other[j] -> other[j-1],
	// direction moving along that face is then other[j] -> other[j+1]
	Point3 dirOnTouchingFace(const Face& other) const {
		for (int i = 0; i < numPoints(); i++) {
			const Point3& p1 = at(i);
			const Point3& p2 = at(i + 1);
			for (int j = 0; j < other.numPoints(); j++) {
				const Point3& otherP1 = other.at(j);
				if (p1 != otherP1) continue;

				const Point3& otherP2 = other.at(j - 1);
				if (p2 == otherP2) {
					const Point3& nextPoint = other.at(j + 1);
					return Line(otherP1, nextPoint).asVector().normalise();
				}
			}
		}
		throw std::invalid_argument("Did not find a matching line!");
	}

	/*
	Arbitrary 90 degree rotation around a normalised axis and point:
		- subtract origin point (a point on the line)
		- rotate around axis (x, y, z): https://en.wikipedia.org/wiki/Rotation_matrix#Rotation_matrix_from_axis_and_angle
		 [ x^2       xy - z    xz + y ]  [p_x]   [ p_x x^2 + p_y (xy - z) + p_z (xz + y) ]
		 [ xy + z    y^2       yz - x ]  [p_y] = [ p_x (xy + z) + p_y y^2 + p_z (yz - x) ]
		 [ xz - y    yz + x    z^2    ]  [p_z]   [ p_x (xz - y) + p_y (yz + x) + p_z z^2 ]
	*/
	Face turnAround(const Line& line) const {
		const Point3 normalisedDir = line.asVector().normalise();
		int x = normalisedDir.x, y = normalisedDir.y, z = normalisedDir.z;
		const Point3& rotationOrigin = line.from;

		std::vector<Point3> newPoints;
		for (int i = 0; i < points.size(); i++) {
			const Point3 p = points[i] - rotationOrigin;
			const Point3 rotatedPoint = {
			p.x*x*x + p.y*(x*y - z) + p.z*(x*z + y),
			p.x*(x*y + z) + p.y*y*y + p.z*(y*z - x),
			p.x*(x*z - y) + p.y*(y*z + x) + p.z*z*z
			};
			newPoints.push_back(rotationOrigin + rotatedPoint);
		}
		return { newPoints };
	}
};

inline std::ostream& operator<<(std::ostream& stream, Face const& f) {
	stream << "{";
	for (int i = 0; i < f.numPoints(); i++) {
		stream << f.points.at(i);
		if (i != f.numPoints() - 1) stream << ", ";
	}
	return stream << "}";
}

