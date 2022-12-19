#pragma once
#include "Convex_Hull.h"
#include "Point_Functions.h"
class Orientation_Geometry
{
private:

	ON_3dPoint m_center;				// The Center Point of the image (second point of input poly)
	ON_3dVector m_vectorRight;			// Unit Vector from center to the right (first point of input poly)
	ON_3dVector m_vectorUp;				// Unit Vector from center to the right (third point of input poly)
	ON_3dVector m_vectorOrthogonal;		// Unit Vector orthogonal to right and up vector from center

	ON_Polyline m_polyRepresentation;	// The Polyline Representation of the orientation geometry (right point to center to up -> 3 points)

public:

	ON_3dPoint	center()			const { return m_center; }
	ON_3dVector vectorRight() 		const { return m_vectorRight; }
	ON_3dVector vectorUp() 			const { return m_vectorUp; }
	ON_3dVector vectorOrthogonal()	const { return m_vectorOrthogonal; }
	ON_Polyline polyRepresentation()const { return m_polyRepresentation; }

	//Orientation_Geometry()  {};

	// Create orientation geometry from curve. Only polylines are allowed and only the first three points are used for construction
	Orientation_Geometry(ON_Curve* orientPoly) 
	{
		const ON_PolylineCurve* polyCurve = ON_PolylineCurve::Cast(orientPoly);
		const ON_Polyline& polygon = polyCurve->m_pline; // Copy Polyline Points by Reference
		if (!polygon) {
			RhinoApp().Print("Orientation_Geometry: Can't get Polyline Points from curve! Input is no polyline!\n");
			throw("Orientation_Geometry: Can't get Polyline Points from curve! Input is no polyline!");
		}
		const int polyPointCount = polygon.Count();

		if (polyPointCount < 3)
		{
			RhinoApp().Print("Orientation_Geometry: Orientation Poly has too few points!\n");
			throw("Orientation_Geometry: Orientation Poly has too few points!");
		}

		const ON_3dPoint* polyArray = polygon.Array();

		// Get Center Point
		m_center = polyArray[1];

		// Create Vectors
		double vectorRightArray[3] = { polyArray[0].x - m_center.x, polyArray[0].y - m_center.y, polyArray[0].z - m_center.z };
		double vectorUpArray[3]    = { polyArray[2].x - m_center.x, polyArray[2].y - m_center.y, polyArray[2].z - m_center.z };
		m_vectorRight	= ON_3dVector(vectorRightArray);
		m_vectorUp		= ON_3dVector(vectorUpArray);

		// Unitize them
		if (!m_vectorRight.Unitize()) {
			RhinoApp().Print("Orientation_Geometry: Vector Right could not be unitized! Is Zero!\n");
			throw("Orientation_Geometry: Vector Right could not be unitized! Is Zero!");
		}
		if (!m_vectorUp.Unitize()) {
			RhinoApp().Print("Orientation_Geometry: Vector Up could not be unitized! Is Zero!\n");
			throw("Orientation_Geometry: Vector Up could not be unitized! Is Zero!");
		}

		// Create Vector Orthogonal to Right and Up
		m_vectorOrthogonal = ON_3dVector::CrossProduct(m_vectorRight, m_vectorUp);

		if (!m_vectorOrthogonal.Unitize()) {
			RhinoApp().Print("Orientation_Geometry: Vector Orthogonal could not be unitized! Is Zero!\n");
			throw("Orientation_Geometry: Vector Orthogonal could not be unitized! Is Zero!");
		}

		// Create Poly Representation
		m_polyRepresentation.SetCapacity(3);
		m_polyRepresentation.SetCount(3);

		m_polyRepresentation[0] = polyArray[0];
		m_polyRepresentation[1] = polyArray[1];
		m_polyRepresentation[2] = polyArray[2];
	}

	void Translate(ON_3dVector transVector) {

		if (m_center.IsValid()) {
			m_center.x += transVector.x;
			m_center.y += transVector.y;
			m_center.z += transVector.z;
		}
		if (m_polyRepresentation.IsValid()) {
			m_polyRepresentation.Translate(transVector);
		}
	}

	void Transform(ON_Xform xform) {
		if (m_center.IsValid()) {
			m_center.Transform(xform);
		}
		if (m_polyRepresentation.IsValid()) {
			m_polyRepresentation.Transform(xform);
		}

	}

	// Get Vector orthogonal to geometry scaled by value
	ON_3dVector GetRaisingVector(const double scaleValue) {
		ON_3dVector raisingVector(m_vectorOrthogonal);
		raisingVector.x *= scaleValue;
		raisingVector.y *= scaleValue;
		raisingVector.z *= scaleValue;
		return raisingVector;
	}

	// Creates an Xform to align the geometries XYZ axis to to World Axis -> Rotate into World plane
	void CreateXFormToWorldAxisAlign(ON_Xform& xform) 
	{
		xform.Rotation(m_vectorRight, m_vectorUp, m_vectorOrthogonal, ON_3dVector(1, 0, 0), ON_3dVector(0, 1, 0), ON_3dVector(0, 0, 1));
	}

	// Creates a rectangular 3D Bounding Box around orientation geometry represented by the convex hull of the 8 box corners
	void CreateBoundingBox(double width, double height, double depth, ON_3dPointArray& cutPointArray)
	{
		bool UseFallback = false;

		ON_3dPoint currentPoint(m_center);
		std::vector<ConvHull::point> imageCorners;

		// Go to lower lower left point
		Point_Functions::Translate3DPoint(currentPoint, m_vectorRight, -width  / 2);
		Point_Functions::Translate3DPoint(currentPoint, m_vectorUp,    -height / 2);
		Point_Functions::Translate3DPoint(currentPoint, m_vectorOrthogonal, -depth / 2);
		imageCorners.push_back(ConvHull::point(currentPoint.x, currentPoint.y));

		// Go to upper lower left
		Point_Functions::Translate3DPoint(currentPoint, m_vectorOrthogonal, depth);
		imageCorners.push_back(ConvHull::point(currentPoint.x, currentPoint.y));

		// Go to upper lower right
		Point_Functions::Translate3DPoint(currentPoint, m_vectorRight, width);
		imageCorners.push_back(ConvHull::point(currentPoint.x, currentPoint.y));

		// Go to lower lower right
		Point_Functions::Translate3DPoint(currentPoint, m_vectorOrthogonal, -depth);
		imageCorners.push_back(ConvHull::point(currentPoint.x, currentPoint.y));

		// Go to lower upper right
		Point_Functions::Translate3DPoint(currentPoint, m_vectorUp, height);
		imageCorners.push_back(ConvHull::point(currentPoint.x, currentPoint.y));

		// Go to upper upper right
		Point_Functions::Translate3DPoint(currentPoint, m_vectorOrthogonal, depth);
		imageCorners.push_back(ConvHull::point(currentPoint.x, currentPoint.y));

		// Go to upper upper left
		Point_Functions::Translate3DPoint(currentPoint, m_vectorRight, -width);
		imageCorners.push_back(ConvHull::point(currentPoint.x, currentPoint.y));

		// Go to lower upper left
		Point_Functions::Translate3DPoint(currentPoint, m_vectorOrthogonal, -depth);
		imageCorners.push_back(ConvHull::point(currentPoint.x, currentPoint.y));

		// Convex Hull of the image box
		std::vector<ConvHull::point> hull = ConvHull::quickHull(imageCorners);

		if (hull.size() > 8) {
			RhinoApp().Print("Convex Hull has more than 8 Points! Falling back to box corners which may give bad results!\n");
			UseFallback= true;
		}
		if (hull.size() < 3) { 
			RhinoApp().Print("Convex Hull has less than 3 Points! Falling back to box Corners which may give bad results!\n");
			UseFallback = true;
		}

		if (UseFallback) {
			for (int i = 0; i < imageCorners.size(); i++) {
				cutPointArray.Append(ON_3dPoint(imageCorners[i].x, imageCorners[i].y, 0));
			}
		}
		else {
			for (int i = 0; i < hull.size(); i++) {
				cutPointArray.Append(ON_3dPoint(hull[i].x, hull[i].y, 0));
			}
		}
	}

};

