#pragma once

class Point_Functions {

public:
	static inline void Translate3DPoint(ON_3dPoint*& point, const ON_3dVector& transVector, const double& scale) {
		point->x += transVector.x * scale;
		point->y += transVector.y * scale;
		point->z += transVector.z * scale;
	}

	static inline void Translate3DPoint(ON_3dPoint& point, const ON_3dVector& transVector, const double& scale) {
		point.x += transVector.x * scale;
		point.y += transVector.y * scale;
		point.z += transVector.z * scale;
	}

	static inline bool isPointInPolygon(const ON_3dPoint* const& polyArray, const ON_3dPoint& testPoint, const int& polyLength) {
		//A point is in a polygon if a line from the point to infinity crosses the polygon an odd number of times
		bool odd = false;

		//For each edge (In this case for each point of the polygon and the previous one)
		for (int i = 0, j = polyLength - 1; i < polyLength; i++) { // Starting with the edge from the last to the first node
			//If a line from the point into infinity crosses this edge
			// One point needs to be above, one below our y coordinate
			if (((polyArray[i].y > testPoint.y) != (polyArray[j].y > testPoint.y))

				// ...and the edge doesn't cross our Y coordinate before our x coordinate (but between our x coordinate and infinity)
				&& (testPoint.x < (polyArray[j].x - polyArray[i].x) * (testPoint.y - polyArray[i].y) / (polyArray[j].y - polyArray[i].y) + polyArray[i].x)) {

				// Invert odd
				odd = !odd;
			}
			j = i;
		}

		//If the number of crossings was odd, the point is in the polygon
		return odd;
	}
};