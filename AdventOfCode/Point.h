#pragma once

#include <compare>

class Point {
public:
	size_t x;
	size_t y;

	Point();
	Point(const size_t& x, const size_t& y);

	bool operator==(const Point&) const = default;

	auto operator<=>(const Point&) const = default;
};
