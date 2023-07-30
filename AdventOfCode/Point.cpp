#include "Point.h"


template<class T> Point<T>::Point() : Point(0, 0) {}
template<class T> Point<T>::Point(const T& x, const T& y) : x(x), y(y) {}
template<class T> Point<T> Point<T>::operator+(const Point<T>& p) {
	return Point<T>(x + p.x, y + p.y);
}
