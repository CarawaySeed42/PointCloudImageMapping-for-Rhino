#include "stdafx.h"
#include "PointCloud_Functions.h"
#include "Point_Functions.h"

// Returns normals of point cloud. Normals will be allocated if they do not exist and specified in argument. Throws "bad_alloc" if normals can not be allocated
bool PointCloud_Functions::GetPointCloudNormals(ON_PointCloud* pCloud, const bool createIfNotExist, ON_3dVector*& pcNormals)
{
	int32_t num_pcPts = pCloud->PointCount();
	bool hasNormals = pCloud->HasPointNormals();

	if (hasNormals) {
		pcNormals = pCloud->m_N.Array();
	}
	else {
		if (createIfNotExist) {
			pCloud->m_N.SetCapacity(num_pcPts);
			pCloud->m_N.SetCount(num_pcPts);
			pcNormals = pCloud->m_N.Array();
			if (nullptr == pcNormals) { pCloud->Destroy(); throw std::bad_alloc{}; }
		}
		else { pcNormals = nullptr; }
	}

	return pCloud->HasPointNormals();
}

void PointCloud_Functions::PointsInPoly(ON_PointCloud* pCloud, const ON_3dPointArray& polyPointArray, bool* isPoinInPoly)
{

	const ON_3dPoint* pointArray = polyPointArray.Array();
	int polyCount = polyPointArray.Count();

	size_t pCloudCount = pCloud->PointCount();
	ON_3dPoint* cloudPoints = pCloud->m_P.Array();

	for (size_t i = 0; i < pCloudCount; ++i) 
	{
		isPoinInPoly[i] = Point_Functions::isPointInPolygon(pointArray, cloudPoints[i], polyCount);
	}
}

void PointCloud_Functions::createPointOnlyCloud(ON_PointCloud* pCloudSource, ON_PointCloud* pCloudTarget, const bool* pointsToCopy, const int pointsToCopyCount) {

	const size_t num_pcPoints = pCloudSource->PointCount();

	// Input Cloud Points content
	ON_3dPoint* pcPoints = pCloudSource->m_P.Array();


	// Initialize Output
	pCloudTarget->m_P.SetCapacity(pointsToCopyCount);
	ON_3dPoint* outPCPoints = pCloudTarget->m_P.Array();

	// Copy Points if true at "to copy index"
	unsigned int counter = 0;
	for (int i = 0; i < num_pcPoints; ++i) {

		if (pointsToCopy[i]) {
			outPCPoints[counter] = pcPoints[i];
			++counter;
		}
	}

	// Set Count of points for output
	pCloudTarget->m_P.SetCount(pointsToCopyCount);
}
