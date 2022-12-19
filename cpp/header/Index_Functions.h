#pragma once
#include <vector>
#include <unordered_set>
class Index_Functions {

public:
	static void BoolArrayToIndexArray(bool* boolArray, size_t boolArraySize, std::vector<size_t>& outIndexVector)
	{
		size_t boolSum = 0;

		for (size_t i = 0; i < boolArraySize; ++i)
		{
			boolSum += boolArray[i];
		}

		outIndexVector.resize(boolSum);
		size_t currentIndex = 0;

		for (size_t i = 0; i < boolArraySize; ++i)
		{
			if (boolArray[i])
			{
				outIndexVector[currentIndex] = i;
				++currentIndex;
			}
		}

	}

	static void IndexArraySubSelect(std::vector<size_t>& inIndexVector, bool* boolArray, std::vector<size_t>& outIndexVector)
	{
		size_t boolSum = 0;
		size_t boolArraySize = inIndexVector.size();

		for (size_t i = 0; i < boolArraySize; ++i)
		{
			boolSum += boolArray[i];
		}

		outIndexVector.resize(boolSum);
		size_t currentIndex = 0;

		for (size_t i = 0; i < boolArraySize; ++i)
		{
			if (boolArray[i])
			{
				outIndexVector[currentIndex] = inIndexVector[i];
				++currentIndex;
			}
		}
	}

};
