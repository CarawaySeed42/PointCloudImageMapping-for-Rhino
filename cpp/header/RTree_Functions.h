#pragma once
#ifndef RTree_Functions_H
#define RTree_Functions_H

class RTree_Functions
{

public:
	static void   InsertPointCloudIntoRTree(ON_RTree& pcTree, const ON_PointCloud* pcPointer, const bool showProgress = 0);
	static size_t InsertPointCloudSelectionIntoRTree(ON_RTree& pcTree, const ON_PointCloud* pcPointer, const bool* toBeInserted, const std::vector<size_t>& indexVector, const bool showProgress);
	static size_t InsertPointCloudinBBIntoRTree(ON_RTree& pcTree, const ON_PointCloud* pcPointer, const ON_3dPoint pointMin, const ON_3dPoint pointMax, const std::vector<size_t>& indexVector, const bool showProgress);
};

class pcRTreeSearchResult
{
public:
	static bool resultCallback(void* a_context, INT_PTR a_idA);
	std::vector<size_t> m_fids;

	pcRTreeSearchResult() {}

	pcRTreeSearchResult(size_t reserveSize) {
		m_fids.reserve(reserveSize);
	}
};

#endif

