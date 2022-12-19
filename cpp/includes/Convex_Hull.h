#pragma once
#ifndef CONVEXHULL_H
#define CONVEXHULL_H
// The following Code was created by MiguelVieira (https://github.com/MiguelVieira/ConvexHull2D)
// Modifications by CarawaySeed42 include:
//	- Raise Precision from single to double precision
//	- Wrapped in class named ConvHull
//	- Removed Main and Print methods

/*
The MIT License (MIT)

Copyright (c) 2014 MiguelVieira

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// Implementations of various convex hull algorithms 
// using the C++ Standard Library.
// For clarity, the implementations do not check for
// duplicate or collinear points.


#include <algorithm>
#include <iostream>
#include <vector>


class ConvHull {
public:

	struct point {
		double x;
		double y;

		point(double xIn, double yIn) : x(xIn), y(yIn) { }
	};

	// The z-value of the cross product of segments 
	// (a, b) and (a, c). Positive means c is ccw
	// from (a, b), negative cw. Zero means its collinear.
	static double ccw(const point& a, const point& b, const point& c) {
		return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
	}

	// Returns true if a is lexicographically before b.
	static bool isLeftOf(const point& a, const point& b) {
		return (a.x < b.x || (a.x == b.x && a.y < b.y));
	}

	// Used to sort points in ccw order about a pivot.
	struct ccwSorter {
		const point& pivot;

		ccwSorter(const point& inPivot) : pivot(inPivot) { }

		bool operator()(const point& a, const point& b) {
			return ccw(pivot, a, b) < 0;
		}
	};

	// The length of segment (a, b).
	static double len(const point& a, const point& b) {
		return sqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y));
	}

	// The unsigned distance of p from segment (a, b).
	static double dist(const point& a, const point& b, const point& p) {
		return fabs((b.x - a.x) * (a.y - p.y) - (b.y - a.y) * (a.x - p.x)) / len(a, b);
	}

	// Returns the index of the farthest point from segment (a, b).
	static size_t getFarthest(const point& a, const point& b, const std::vector<point>& v) {
		size_t idxMax = 0;
		double distMax = dist(a, b, v[idxMax]);

		for (size_t i = 1; i < v.size(); ++i) {
			double distCurr = dist(a, b, v[i]);
			if (distCurr > distMax) {
				idxMax = i;
				distMax = distCurr;
			}
		}

		return idxMax;
	}


	// The gift-wrapping algorithm for convex hull.
	// https://en.wikipedia.org/wiki/Gift_wrapping_algorithm
	static std::vector<point> giftWrapping(std::vector<point> v) {
		// Move the leftmost point to the beginning of our vector.
		// It will be the first point in our convext hull.
		std::swap(v[0], *std::min_element(v.begin(), v.end(), isLeftOf));

		std::vector<point> hull;
		// Repeatedly find the first ccw point from our last hull point
		// and put it at the front of our array. 
		// Stop when we see our first point again.
		do {
			hull.push_back(v[0]);
			std::swap(v[0], *min_element(v.begin() + 1, v.end(), ccwSorter(v[0])));
		} while (v[0].x != hull[0].x && v[0].y != hull[0].y);

		return hull;
	}


	// The Graham scan algorithm for convex hull.
	// https://en.wikipedia.org/wiki/Graham_scan
	static std::vector<point> GrahamScan(std::vector<point> v) {
		// Put our leftmost point at index 0
		std::swap(v[0], *min_element(v.begin(), v.end(), isLeftOf));

		// Sort the rest of the points in counter-clockwise order
		// from our leftmost point.
		sort(v.begin() + 1, v.end(), ccwSorter(v[0]));

		// Add our first three points to the hull.
		std::vector<point> hull;
		auto it = v.begin();
		hull.push_back(*it++);
		hull.push_back(*it++);
		hull.push_back(*it++);

		while (it != v.end()) {
			// Pop off any points that make a convex angle with *it
			while (ccw(*(hull.rbegin() + 1), *(hull.rbegin()), *it) >= 0) {
				hull.pop_back();
			}
			hull.push_back(*it++);
		}

		return hull;
	}


	// The monotone chain algorithm for convex hull.
	static std::vector<point> monotoneChain(std::vector<point> v) {
		// Sort our points in lexicographic order.
		std::sort(v.begin(), v.end(), isLeftOf);

		// Find the lower half of the convex hull.
		std::vector<point> lower;
		for (auto it = v.begin(); it != v.end(); ++it) {
			// Pop off any points that make a convex angle with *it
			while (lower.size() >= 2 && ccw(*(lower.rbegin() + 1), *(lower.rbegin()), *it) >= 0) {
				lower.pop_back();
			}
			lower.push_back(*it);
		}

		// Find the upper half of the convex hull.
		std::vector<point> upper;
		for (auto it = v.rbegin(); it != v.rend(); ++it) {
			// Pop off any points that make a convex angle with *it
			while (upper.size() >= 2 && ccw(*(upper.rbegin() + 1), *(upper.rbegin()), *it) >= 0) {
				upper.pop_back();
			}
			upper.push_back(*it);
		}

		std::vector<point> hull;
		hull.insert(hull.end(), lower.begin(), lower.end());
		// Both hulls include both endpoints, so leave them out when we 
		// append the upper hull.
		hull.insert(hull.end(), upper.begin() + 1, upper.end() - 1);
		return hull;
	}


	// Recursive call of the quickhull algorithm.
	static void quickHull(const std::vector<point>& v, const point& a, const point& b,
		std::vector<point>& hull) {
		if (v.empty()) {
			return;
		}

		point f = v[getFarthest(a, b, v)];

		// Collect points to the left of segment (a, f)
		std::vector<point> left;
		for (auto p : v) {
			if (ccw(a, f, p) > 0) {
				left.push_back(p);
			}
		}
		quickHull(left, a, f, hull);

		// Add f to the hull
		hull.push_back(f);

		// Collect points to the left of segment (f, b)
		std::vector<point> right;
		for (auto p : v) {
			if (ccw(f, b, p) > 0) {
				right.push_back(p);
			}
		}
		quickHull(right, f, b, hull);
	}

	// QuickHull algorithm. 
	// https://en.wikipedia.org/wiki/QuickHull
	static std::vector<point> quickHull(const std::vector<point>& v) {
		std::vector<point> hull;

		// Start with the leftmost and rightmost points.
		point a = *std::min_element(v.begin(), v.end(), isLeftOf);
		point b = *std::max_element(v.begin(), v.end(), isLeftOf);

		// Split the points on either side of segment (a, b)
		std::vector<point> left, right;
		for (auto p : v) {
			ccw(a, b, p) > 0 ? left.push_back(p) : right.push_back(p);
		}

		// Be careful to add points to the hull
		// in the correct order. Add our leftmost point.
		hull.push_back(a);

		// Add hull points from the left (top)
		quickHull(left, a, b, hull);

		// Add our rightmost point
		hull.push_back(b);

		// Add hull points from the right (bottom)
		quickHull(right, b, a, hull);

		return hull;
	}

	static std::vector<point> getPoints() {
		std::vector<point> v;

		const double lo = -100.0;
		const double hi = 100.0;

		for (int i = 0; i < 100; ++i) {
			double x = lo +
				static_cast<double>(
					rand()) / static_cast<double>(RAND_MAX / (hi - lo));

			double y = lo +
				static_cast<double>(
					rand()) / static_cast<double>(RAND_MAX / (hi - lo));

			v.push_back(point(x, y));
		}

		return v;
	}
};
#endif