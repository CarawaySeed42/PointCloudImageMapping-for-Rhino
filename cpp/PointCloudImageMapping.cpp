#include "stdafx.h" 
#include <chrono>
#include <math.h>
#include <comdef.h>
#include <omp.h>
#include "FreeImage.h"
#include "Array2D.h"
#include "ImageWrapper.h"
#include "Orientation_Geometry.h"
#include "RTree_Functions.h"
#include "PointCloud_Functions.h"
#include "Point_Functions.h"
#include "Index_Functions.h"
#include "RhinoProgress.h"

// These are critical for creation of the DLL.
#define DLLEXPORT extern "C" __declspec(dllexport)
#define UCLASS()
#define GENERATED_BODY()
#define UFUNCTION()

//
// For Rhino C++ examples see:
// https://developer.rhino3d.com/guides/cpp/
//

// Choose the approach here. Distance based calculates the point colors through distance from image corner. 
// RTree creates an RTree and searches through the pixel grid
// Distance based is multi threaded, RTree Approach is not
#define DistanceBasedApproach 1
#define RTreeApproach 0

float defaultArgFloat = -999.0;
/// <summary>
/// Map image to a point cloud with specified witdth. Height will be determined from aspect ratio.
/// </summary>
/// <param name="pCloud"></param>
/// <param name="orientPoly"></param>
/// <param name="fileWideString"></param>
/// <param name="width"></param>
/// <param name="depth"></param>
/// <param name="mappingSettings"></param>
/// <param name="raiseValue"></param>
/// <param name="td"></param>
/// <returns>success: True if successful, False if error encountered</returns>
DLLEXPORT bool PointCloudImageMapping(ON_PointCloud* pCloud, ON_Curve* orientPoly, LPCWSTR fileWideString, double width, double depth = 0.2, unsigned char mappingSettings = 4, double raiseValue = 0.02, float& td = defaultArgFloat) {
	
	// Set maximum number of treads
	int maxThreadCount = 8;

	// Convert LPCWSTR to const char*
	_bstr_t b(fileWideString);
	const char* fileString = b;

	// Decode settings
	bool colorMapping	= (mappingSettings >> 2) & 1;
	bool greyscale		= (mappingSettings >> 1) & 1;
	bool raised			= mappingSettings & 1;

	// Sanitize input
	if (width < 0)	{ width = abs(width); }
	if (width == 0) { return false; }
	if (!colorMapping && !raised) { RhinoApp().Print("PointCloudImageMapping: Neither color mapping nor raised set! Defaulting to colorMapping!\n"); colorMapping = true; }

	// Declarations and Initializations
	std::chrono::steady_clock::time_point time0, time1;		// Timer
	time0 = std::chrono::steady_clock::now();				// Get Start Time
	ON_3dPoint* pcPoints;									// Pointer to Point Cloud Points of input cloud
	ON_Color* pcColors;										// Pointer to Point Cloud Colors of input cloud
	ON_3dVector* pcNormals = nullptr;						// Pointer to Point Cloud Normals of input cloud
	const bool saveOldColors = true;						// Should the previous pc colors be saved to cloud normals?
	bool hasNormals;										// Does the input cloud have normals?

	double height;											// height of image in rhino units (e.g. meters) instead of pixels
	bool isImageTransparent = false;						// States if Image has Transparencies
	double alphaWeightImage = 1;							// Alpha fraction to weight color with. Is 1 if no transparencies
	double alphaWeightPCPoint = 0;							// Fraction of the original Cloud Point that goes into the new color (0 if image fully opaque)
	double greyValueScale = 1;								// Grey Value used for height sclaing during height mapping
	

	RGBQUAD mapColor;										// Current Pixel Color for point mapping
	ON_Color* pColor;										// Pointer to the current Point Cloud Point's Color, which a new color is assigned to
	size_t pcPointIndex;									// Index to the current Point that should be raised
	std::unordered_set<size_t> processedIndices;			// Indices of Points that have been processed. For height mapping we can't map to a point multiple times. 
															// This might happen due to points being on the border between pixels

	// RTree Search Result
	pcRTreeSearchResult results(10000);						// Result vector for rTree Search with initial guess for reserved size

	// Load Image
	RhinoProgress::PromptMessage("Loading Image...");
	RhinoApp().Print("Loading Image: %s\n", fileString);
	ImageWrapper image(fileString);

	if (!image.ImageLoaded()) { RhinoApp().Print("Image: %s could not be loaded\n", fileString);  return false; }
	if (!image.IsValid())	{ RhinoApp().Print("Data form image is not valid to be processed!\n", fileString);  return false; }

	const size_t imageWidth = image.ImgWidth();				// Width of image in Pixels
	const size_t imageHeight = image.ImgHeight();			// Height of image in Pixels

	// Get image height in Rhino units and get transparency status
	height = width / image.AspectRatio(); 
	isImageTransparent = image.IsTransparent();

	// Image Corner from origin and the size of each pixel in Rhino units 
	const double lowerLeftCord[2] = { -width / 2, -height / 2 };
	const double distancePerPixel = width / imageWidth;

	// Point Cloud Preparation
	const int32_t pcCount = pCloud->PointCount();
	pcPoints = pCloud->m_P.Array();

	if (!(pCloud->HasPointColors())) {
		pCloud->m_C.SetCapacity(pcCount);
		pCloud->m_C.SetCount(pcCount);
	}
	pcColors = pCloud->m_C.Array();
	if (nullptr == pcColors) { pCloud->Destroy(); RhinoApp().Print("Could not allocate Point Cloud Colors! Cancel Operation\n"); return false; }
	
	// Set Orientation Properties from orientatation poly argument
	Orientation_Geometry orientGeo(orientPoly);

	// PreCut Point Cloud with poly
	RhinoProgress::PromptMessage("Cut Point Cloud outside image plane...");
	std::unique_ptr<bool[]>  inPolyBool(new bool[pCloud->PointCount()]);
	
	ON_3dPointArray cutGeo;
	orientGeo.CreateBoundingBox(width, height, depth, cutGeo);
	PointCloud_Functions::PointsInPoly(pCloud, cutGeo, inPolyBool.get());

	// Create a new Cloud with only Point Coordinates, so no colors or normals
	int pointInPolyCount = 0;
	for (size_t i = 0; i < pcCount; ++i) { pointInPolyCount += inPolyBool[i]; }
	if (pointInPolyCount == 0) { RhinoApp().Print("There are no point cloud points in image area! Cancelling!\n", fileString);  return false; }

	ON_PointCloud partialCloud;
	PointCloud_Functions::CreatePointOnlyCloud(pCloud, &partialCloud, inPolyBool.get(), pointInPolyCount);
	
	// Get Indices of Points in Poly in original Cloud and then free bool array because it is not needed anymore
	std::vector<size_t> origIndexVector;
	Index_Functions::BoolArrayToIndexArray(inPolyBool.get(), pCloud->PointCount(), origIndexVector);
	inPolyBool.reset();

	// Move Cloud to origin 
	RhinoProgress::PromptMessage("Move and Rotate Cloud into Image plane...");
	partialCloud.Translate(ON_3dVector(-orientGeo.center().x, -orientGeo.center().y, -orientGeo.center().z));

	// Rotate Cloud into image plane
	ON_Xform worldAxisAlignXForm;
	orientGeo.CreateXFormToWorldAxisAlign(worldAxisAlignXForm);
	partialCloud.Transform(worldAxisAlignXForm);

	// Vector to raise the point cloud if raising is set
	ON_3dVector raiseVector = orientGeo.GetRaisingVector(raiseValue);



#if RTreeApproach

	// Cut the partial cloud by Bounding Box to have only points left that will be colored
	ON_3dPoint pointMin(-width / 2, -height / 2, -depth / 2);
	ON_3dPoint pointMax(width / 2, height / 2, depth / 2);

	// Insert Points into RTree
	RhinoApp().Print("Create RTree for pixel mapping...\n");
	ON_RTree rTree;
	size_t rTreeCount = RTree_Functions::InsertPointCloudinBBIntoRTree(rTree, &partialCloud, pointMin, pointMax, origIndexVector, true);

	RhinoApp().Print("RTree contains %d Points!\n", rTreeCount);

	// If no points in tree then return, otherwise reserve count entries for our unordered_set
	if (rTreeCount == 0) { return false; }
	processedIndices.reserve(rTreeCount);

	partialCloud.~ON_PointCloud();	// Call destructor of partial cloud

	/* The Mapping loop... */
	double a_min[2] = { 0,0 };
	double a_max[2] = { 0,0 };
	const size_t progInterval = (image.ImgWidth() / 20) + 1; // Prompt Progress Interval, Standard is every 5% of columns

	for (size_t i = 0; i < image.ImgWidth(); ++i)
	{
		a_min[0] = lowerLeftCord[0] + i * distancePerPixel;
		a_max[0] = lowerLeftCord[0] + (i + 1) * distancePerPixel;

		if (i % progInterval == 0) {
			RhinoProgress::PromptProgress("Map image to Point Cloud Colors...", i, image.ImgWidth());
		}
		
		for (size_t j = 0; j < image.ImgHeight(); ++j)
		{
			// Current Pixel Color
			mapColor = image.pixelArray[i][j];

			// Check if we have to scale the color with its' transparency
			if (isImageTransparent) {
				if (mapColor.rgbReserved == 0) {				// If Pixel is fully transparent then there is no color to map and we can go to the next pixel
					continue;					
				}

				alphaWeightImage = static_cast<double>(mapColor.rgbReserved) / 255.0;	// Weight pixel color with alpha (0 means transparency here, 255 is opaque)
				alphaWeightPCPoint = 1.0 - alphaWeightImage;						// The difference to full weight will be given to the original pc point color
			}

			// Clear old Search Results
			results.m_fids.resize(0);
			
			// Search all points that the pixels color will be assigned to
			a_min[1] = lowerLeftCord[1] + j * distancePerPixel;
			a_max[1] = lowerLeftCord[1] + (j + 1) * distancePerPixel;
			rTree.Search2d(a_min, a_max, pcRTreeSearchResult::resultCallback, &results);
			
			// Assign new color if colored argument was true
			if (colorMapping) 
			{
				// Turn Greyscale if necessary
				if (greyscale) { ImageWrapper::RGBQUADToGreyscale(mapColor); }

				for (int h = 0; h < results.m_fids.size(); h++) {
					pColor = &pcColors[results.m_fids[h]];
					*pColor = ON_Color(static_cast<int>(mapColor.rgbRed   * alphaWeightImage + alphaWeightPCPoint * pColor->Red()),   \
									   static_cast<int>(mapColor.rgbGreen * alphaWeightImage + alphaWeightPCPoint * pColor->Green()), \
								       static_cast<int>(mapColor.rgbBlue  * alphaWeightImage + alphaWeightPCPoint * pColor->Blue()));
				}
			}

			// Raise cloud point by specified value if raised argument was true
			if (raised)
			{
				// Get Greyscale in any case. Raising will be done with greyscale value and alpha value
				greyValueScale = (ImageWrapper::RGBQUADToGreyscaleDouble(mapColor) / 255.0) * alphaWeightImage;

				for (size_t h = 0; h < results.m_fids.size(); h++) {

					pcPointIndex = results.m_fids[h];

					// Shift point if it has not been shifted before. This might happen if point is on pixel boundary. Insert index into set to keep track of processed points
					if (processedIndices.find(pcPointIndex) == processedIndices.end()) {

						Point_Functions::Translate3DPoint(pcPoints[pcPointIndex], raiseVector, greyValueScale);
						processedIndices.insert(pcPointIndex);
					}
				}
			}
		}
	}

#elif DistanceBasedApproach

	// Get distance to every point from lower left corner and x,y scale distances to pixels and normalize z with half depth to later exclude points to high or too low ralative to image plane
	ON_3dVectorArray distanceToLowerLeft;
	{
		ON_3dPoint mainPoint(lowerLeftCord);
		PointCloud_Functions::DistanceTo3DPoint(&partialCloud, mainPoint, distanceToLowerLeft);

		double pixelPerDistance = 1 / distancePerPixel;
		ON_Xform pixelScaler = ON_Xform::DiagonalTransformation(pixelPerDistance, pixelPerDistance, 2 / depth);
		distanceToLowerLeft.Transform(pixelScaler);
	}

	partialCloud.~ON_PointCloud();	// Call destructor of partial cloud

	/* The Mapping loop... */
	int widthIndex = 0;
	int heightIndex = 0;
	size_t iterationCount = distanceToLowerLeft.Count();
	const size_t progInterval = (iterationCount / 20) + 1; // Prompt Progress Interval, Standard is every 5% of columns

	int threadChunksize = (iterationCount / maxThreadCount) + 1;
	omp_set_num_threads(maxThreadCount);

#pragma omp parallel for schedule(static, threadChunksize) private(widthIndex, heightIndex, mapColor, pColor, greyValueScale, pcPointIndex) firstprivate(alphaWeightImage, alphaWeightPCPoint)
	for (int i = 0; i < iterationCount; ++i)
	{
		if (i % progInterval == 0) {
			RhinoProgress::PromptProgress("Map image to Point Cloud Colors...", i, iterationCount);
		}

		widthIndex = static_cast<int>(distanceToLowerLeft[i].x);
		heightIndex = static_cast<int>(distanceToLowerLeft[i].y);

		// Skip if index out of bounds or point higher or lower than depth
		if (widthIndex >= imageWidth || heightIndex > imageHeight) { continue; }
		if (std::abs(distanceToLowerLeft[i].z) > 1) { continue; }

		// Current Pixel Color
		mapColor = image.pixelArray[widthIndex][heightIndex];

		// Check if we have to scale the color with its' transparency
		if (isImageTransparent) {
			if (mapColor.rgbReserved == 0) {				// If Pixel is fully transparent then there is no color to map and we can go to the next pixel
				continue;
			}

			alphaWeightImage = static_cast<double>(mapColor.rgbReserved) / 255.0; // Weight pixel color with alpha (0 means transparency here, 255 is opaque)
			alphaWeightPCPoint = 1.0 - alphaWeightImage;						  // The difference to full weight will be given to the original pc point color
		}

		// Point Index in Cloud
		pcPointIndex = origIndexVector[i];

		// Assign new color if colored argument was true
		if (colorMapping)
		{
			// Turn Greyscale if necessary
			if (greyscale) { ImageWrapper::RGBQUADToGreyscale(mapColor); }

			pColor = &pcColors[pcPointIndex];
			*pColor = ON_Color(static_cast<int>(mapColor.rgbRed * alphaWeightImage + alphaWeightPCPoint * pColor->Red()), \
				static_cast<int>(mapColor.rgbGreen * alphaWeightImage + alphaWeightPCPoint * pColor->Green()), \
				static_cast<int>(mapColor.rgbBlue * alphaWeightImage + alphaWeightPCPoint * pColor->Blue()));
		}

		// Raise cloud point by specified value if raised argument was true
		if (raised)
		{
			// Get Greyscale in any case. Raising will be done with greyscale value and alpha value
			greyValueScale = (ImageWrapper::RGBQUADToGreyscaleDouble(mapColor) / 255.0) * alphaWeightImage;

			// Shift point if it has not been shifted before. This might happen if point is on pixel boundary. Insert index into set to keep track of processed points
			if (processedIndices.find(pcPointIndex) == processedIndices.end()) {

				Point_Functions::Translate3DPoint(pcPoints[pcPointIndex], raiseVector, greyValueScale);
#pragma omp critical
				processedIndices.insert(pcPointIndex);
			}
		}
	}

#endif
	
	time1 = std::chrono::steady_clock::now();
	td = 1e-6f * std::chrono::duration_cast<std::chrono::microseconds> (time1 - time0).count();

	RhinoProgress::PromptMessage("Mapping Completed...");
	RhinoApp().Print("Mapping Completed...\n");

	return true;

}