#pragma once

#include <compare>
#include <boost/container_hash/hash.hpp>

// https://stackoverflow.com/questions/495021/why-can-templates-only-be-implemented-in-the-header-file
// templates effectively generate new classes, which messes with header vs cpp files
template<class T>
class Point {
public:
	T x;
	T y;

	Point() : Point<T>(0, 0) {}
	Point(const T& x, const T& y) : x(x), y(y) {}

	// copy/move constructors are implicit
	//Point(const Point<T>& p) = default;
	//Point(Point<T>&& p) = default;

	bool operator==(const Point<T>&) const = default;

	auto operator<=>(const Point<T>&) const = default;

	Point<T> operator+(const Point<T>& p) const {
		return Point<T>(x + p.x, y + p.y);
	}

	T dot(const Point<T>& p) const {
		return x * p.x + y * p.y;
	}

	const static Point UP;
	const static Point DOWN;
	const static Point LEFT;
	const static Point RIGHT;
};

/*
Point may use int, or int64, but should always have valid casts for -1/0/1 (so not size_t, which is unsigned)
*/
template<class T> const Point<T> Point<T>::LEFT{ -1, 0 };
template<class T> const Point<T> Point<T>::RIGHT{ 1, 0 };
template<class T> const Point<T> Point<T>::UP{ 0, 1 };
template<class T> const Point<T> Point<T>::DOWN{ 0, -1 };

template<class T> inline std::ostream& operator<<(std::ostream& stream, Point<T> const& p) {
	return stream << "(" << p.x << ", " << p.y << ")";
}

// Could equally define as boost::hash, but then have to specify that in container constructors
template<class T> struct std::hash<Point<T>> {
	std::size_t operator()(const Point<T>& p) const {
		std::size_t seed = 0;
		boost::hash_combine(seed, p.x);
		boost::hash_combine(seed, p.y);
		return seed;
	}
};

