#pragma once
// A class containing static methods applicable to ON_PointCloud objects
class PointCloud_Functions
{

public:
	static bool GetPointCloudNormals	(ON_PointCloud* pCloud, const bool createIfNotExist, ON_3dVector*& pcNormals);
	static void PointsInPoly			(ON_PointCloud* pCloud, const ON_3dPointArray& polyPointArray, bool* isPoinInPoly);
	static void CreatePointOnlyCloud	(const ON_PointCloud* pCloudSource, ON_PointCloud* pCloudTarget, const bool* pointsToCopy, const int pointsToCopyCount);
	static void DistanceTo2DPoint		(const ON_PointCloud* pCloud, const ON_2dPoint mainPoint, ON_2dVectorArray& outVectorArray);
	static void DistanceTo3DPoint		(const ON_PointCloud* pCloud, const ON_3dPoint mainPoint, ON_3dVectorArray& outVectorArray);

	static inline void TranslateCloud(ON_PointCloud*& pCloud, const double transVec[3]) {

		ON_3dPoint* pcPoints = pCloud->m_P.Array();
		int pCount = pCloud->PointCount();

		for (int i = 0; i < pCount; ++i) {
			pcPoints[i].x += transVec[0];
			pcPoints[i].y += transVec[1];
			pcPoints[i].z += transVec[2];
		}
	}

	static inline void TranslatePartialCloud( ON_PointCloud*& pCloud, const double transVec[3], bool* isPointSelected ) {

		ON_3dPoint* pcPoints = pCloud->m_P.Array();
		int pCount = pCloud->PointCount();

		for (int i = 0; i < pCount; ++i) {

			if (isPointSelected[i]) {
				pcPoints[i].x += transVec[0];
				pcPoints[i].y += transVec[1];
				pcPoints[i].z += transVec[2];
			}
		}
	}

};

