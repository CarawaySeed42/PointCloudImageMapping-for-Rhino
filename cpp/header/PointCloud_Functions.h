#pragma once
// A class containing static methods applicable to ON_PointCloud objects
class PointCloud_Functions
{

public:
	static bool GetPointCloudNormals	(ON_PointCloud* pCloud, const bool createIfNotExist, ON_3dVector*& pcNormals);
	static void PointsInPoly			(ON_PointCloud* pCloud, const ON_3dPointArray& polyPointArray, bool* isPoinInPoly);
	static void CreatePointOnlyCloud	(const ON_PointCloud* pCloudSource, ON_PointCloud* pCloudTarget, const bool* pointsToCopy, const int pointsToCopyCount);
	static void Pointsin3DBBox			(const ON_PointCloud* pCloudSource, ON_PointCloud* pCloudTarget, const bool* pointsToCopy, const int pointsToCopyCount);
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

	

	static inline void EncodeIntensityColorToUINT64(uint64_t& target_UINT64, const unsigned int& Intensity, const unsigned int& Red, const unsigned int& Green, const unsigned int& Blue)
	{
		// Encode intensity and three Colors into first four bytes of z-Component of normal vector
		target_UINT64 = target_UINT64 >> 32;
		target_UINT64 = (target_UINT64 << 8) | (uint8_t)Intensity;
		target_UINT64 = (target_UINT64 << 8) | (uint8_t)Red;
		target_UINT64 = (target_UINT64 << 8) | (uint8_t)Green;
		target_UINT64 = (target_UINT64 << 8) | (uint8_t)Blue;

		// Set Flag in fifth byte that color was saved
		target_UINT64 = target_UINT64 | 0xFF00000000;
	}

	/// <summary>
	/// Encodes a IntRGB Color value into the Z-component of a Point Cloud normal. The bytes are encoded into the 4 least significant bytes of a 64-bit Integer.
	/// A fifth  byte is also set to indicate that saved colors exist
	/// The resulting double is an identical byte to byte representation of said 64-bit Integer.
	/// If the cloud already has saved colors then nothing will be safed except if forceOverwrite is set.
	/// </summary>
	/// <param name="pcColor :        (Ref) Color which should be encoded into pcNormal."></param>
	/// <param name="pcNormal :       (Ref) The point cloud normal which the colors should be encoded into. Is also output of this function"></param>
	/// <param name="forceOverwrite : Should colors always be saved instead of only when the backup didn't exist?"></param>
	static inline void SaveOriginalPCColors(const ON_Color& pcColor, ON_3dVector& pcNormal, const bool& forceOverwrite)
	{
		// Initialize
		double newVectorX = 0.0; // X-Component of the new normal vector for this point cloud point
		double newVectorY = 0.0; // Y-Component of the new normal vector for this point cloud point
		double newVectorZ = 0.0; // Z-Component of the new normal vector for this point cloud point

		// Used to Encode the color bytes into one double value, so the byte literal would be something like 0B...FlagIntRGB (if a byte literal B existed)
		uint64_t intNewVectorZ = 0;

		newVectorX = pcNormal.x;
		newVectorY = pcNormal.y;
		newVectorZ = pcNormal.z;
		intNewVectorZ = *reinterpret_cast<uint64_t*>(&newVectorZ);

		// Save Color if flag not set yet or if force overwrite
		if (((intNewVectorZ & 0xFF00000000) == 0) || forceOverwrite) {

			// Encode PC Colors into intNewVectorZ
			EncodeIntensityColorToUINT64(intNewVectorZ, pcColor.Red(), pcColor.Red(), pcColor.Green(), pcColor.Blue());

			newVectorZ = *reinterpret_cast<double*>(&intNewVectorZ);			// Convert int back to double and write to normal
			pcNormal = ON_3dVector(newVectorX, newVectorY, newVectorZ);
		}
	}

};

