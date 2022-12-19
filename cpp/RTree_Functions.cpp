#include "stdafx.h"
#include "RTree_Functions.h"
#include "RhinoProgress.h"

void RTree_Functions::InsertPointCloudIntoRTree(ON_RTree& pcTree, const ON_PointCloud* pcPointer, const bool showProgress)
{
	// Get pointer to points in pointcloud and number of points
	const ON_3dPoint* pcPoints = pcPointer->m_P.Array();
	const int num_pcPoints = pcPointer->PointCount();

	double insertMin[3] = { 0, 0, 0 };

	for (size_t i = 0; i < num_pcPoints; ++i)
	{

		if ((i % 100000 == 0) && showProgress) {
			RhinoProgress::PromptProgress("Create RTree", i, num_pcPoints);
		}

		insertMin[0] = pcPoints[i].x; insertMin[1] = pcPoints[i].y; insertMin[2] = pcPoints[i].z;
		pcTree.Insert(insertMin, insertMin, static_cast<int>(i));

	}
}

size_t RTree_Functions::InsertPointCloudSelectionIntoRTree(ON_RTree& pcTree, const ON_PointCloud* pcPointer, const bool* toBeInserted, const std::vector<size_t>& indexVector, const bool showProgress)
{
	// Get pointer to points in pointcloud and number of points
	const ON_3dPoint* pcPoints = pcPointer->m_P.Array();
	const int num_pcPoints = pcPointer->PointCount();
	const size_t insertionCount = indexVector.size();			// Number of Points to be inserted into RTree
	const size_t* pIndexVector = indexVector.data();			// Pointer to raw vector data for no reason

	if (insertionCount == 0) { RhinoApp().Print("InsertPointCloudSelectionIntoRTree: There are no cloud points to insert into RTree!!\n"); return 0; }
	if (num_pcPoints < insertionCount) { RhinoApp().Print("InsertPointCloudSelectionIntoRTree: There are more points requested for insertion than point cloud has!\n"); return 0; }

	double insertMin[2] = { 0, 0 };
	size_t count = 0;

	const size_t progInterval = (static_cast<size_t>(num_pcPoints) / 20) + 1; // Prompt Progress Interval, Standard is every 5%

	for (size_t i = 0; i < num_pcPoints; ++i)
	{
		if (showProgress && (i % progInterval == 0)) {
			RhinoProgress::PromptProgress("Create RTree", i, num_pcPoints);
		}
		if (toBeInserted[i]){
			insertMin[0] = pcPoints[i].x; insertMin[1] = pcPoints[i].y;
			pcTree.Insert2d(insertMin, insertMin, static_cast<int>(pIndexVector[i]));
			++count;
		}
	}

	return count;
}

size_t RTree_Functions::InsertPointCloudinBBIntoRTree(ON_RTree& pcTree, const ON_PointCloud* pcPointer, const ON_3dPoint pointMin, const ON_3dPoint pointMax, const std::vector<size_t>& indexVector, const bool showProgress) {

	// Get pointer to points in pointcloud and number of points
	const ON_3dPoint* pcPoints = pcPointer->m_P.Array();
	const int num_pcPoints = pcPointer->PointCount();
	const size_t insertionCount = indexVector.size();			// Number of Points to be inserted into RTree
	//const size_t* pIndexVector = indexVector.data();			// Pointer to raw vector data for no reason

	if (insertionCount == 0) { RhinoApp().Print("InsertPointCloudSelectionIntoRTree: There are no cloud points to insert into RTree!!\n"); return 0; }
	if (num_pcPoints < insertionCount) { RhinoApp().Print("InsertPointCloudSelectionIntoRTree: There are more points requested for insertion than point cloud has!\n"); return 0; }

	double insertMin[2] = { 0, 0 };
	size_t count = 0;

	const size_t progInterval = (static_cast<size_t>(num_pcPoints) / 20) + 1; // Prompt Progress Interval, Standard is every 5%

	for (size_t i = 0; i < num_pcPoints; ++i)
	{
		if (showProgress && (i % progInterval == 0)) {
			RhinoProgress::PromptProgress("Create RTree", i, num_pcPoints);
		}
		if (pcPoints[i].x > pointMin.x && pcPoints[i].x < pointMax.x && pcPoints[i].y > pointMin.y && pcPoints[i].y < pointMax.y && pcPoints[i].z > pointMin.z && pcPoints[i].z < pointMax.z) {
			insertMin[0] = pcPoints[i].x; insertMin[1] = pcPoints[i].y;
			pcTree.Insert2d(insertMin, insertMin, static_cast<int>(indexVector[i]));
			++count;
		}
	}

	return count;
}

bool pcRTreeSearchResult::resultCallback(void* a_context, INT_PTR a_idA)
{
	pcRTreeSearchResult* pThis = (pcRTreeSearchResult*)a_context;
	pThis->m_fids.push_back((int)a_idA);
	return true;
}

