#pragma once
#include "FreeImage.h"
#include "Array2D.h"

class ImageWrapper
{
	FIBITMAP* imageMap = nullptr;
	size_t imgWidth = 0;
	size_t imgHeight = 0;
	size_t imgBitDepth = 0;
	double aspectRatio = 0;
	bool   isTransparent = false;
	bool   imageLoaded = false;

public:
	Array2D<RGBQUAD>		pixelArray;						//Pixel Array access first image column then row (x,y)
	FREE_IMAGE_COLOR_TYPE	colorType	= FIC_RGB;
	FREE_IMAGE_FORMAT		imageFormat = FIF_UNKNOWN;

	ImageWrapper(const char* filename) 
	{
		// Load Image
		imageFormat = FreeImage_GetFileType(filename);
		imageMap = FreeImage_Load(imageFormat, filename);

		if (!imageMap) { imageLoaded = false; return; }

		imageLoaded = ImportImageMap();
	}

	~ImageWrapper() {
		FreeImage_Unload(imageMap);
		pixelArray.~Array2D();
	}

	size_t		ImgWidth()		 const { return imgWidth; }
	size_t		ImgHeight()		 const { return imgHeight; }
	size_t		ImgBitDepth()	 const { return imgBitDepth; }
	double		AspectRatio()	 const { return aspectRatio; }
	bool		IsTransparent()	 const { return isTransparent; }
	bool		ImageLoaded()	 const { return imageLoaded; }

	/// <summary>
	/// Returns if object is valid. ImageWrapper object is valid if the imageMap is loaded and the pixelArray is valid and pixel rows and columns are bigger than zero
	/// </summary>
	/// <returns>isValid (bool)</returns>
	bool IsValid() { return (nullptr != imageMap && pixelArray.isValid() && pixelArray.get_cols() > 0 && pixelArray.get_rows() > 0); }

	/// <summary>
	/// Import Image information and 2D Pixel Array from FIBITMAP member. Information will be written to this object's members
	/// </summary>
	/// <returns>success (bool)</returns>
	bool ImportImageMap() {

		if (!imageMap) { imageLoaded = false; return false; }

		// Get Image Informations
		imgWidth = FreeImage_GetWidth(imageMap);
		imgHeight = FreeImage_GetHeight(imageMap);
		aspectRatio = (double)imgWidth / (double)imgHeight;

		imgBitDepth = FreeImage_GetBPP(imageMap);
		isTransparent = FreeImage_IsTransparent(imageMap);
		colorType = FreeImage_GetColorType(imageMap);

		// Create Pixel Array2D
		try {
			pixelArray.create(imgWidth, imgHeight);
		}
		catch (...)
		{
			imageLoaded = false;
			return false;
		}

		const size_t pixelCountWidth = pixelArray.get_rows();
		const size_t pixelCountHeight = pixelArray.get_cols();

		// Return if pixel Array is too small to hold pixels from image
		if ((pixelCountWidth < imgWidth) || (pixelCountHeight < imgHeight)) {
			imageLoaded = false;
			return false;
		}

		// Map Image Data to Pixel Array
		for (size_t i = 0; i < pixelCountWidth; ++i) {
			for (size_t j = 0; j < pixelCountHeight; ++j)
			{
				FreeImage_GetPixelColor(imageMap, static_cast<unsigned int>(i), static_cast<unsigned int>(j), &pixelArray[i][j]);
			}
		}

		imageLoaded = true;
		return true;
	}

	/// <summary>
	/// In place conversion of the pixelArray member of ImageWrapper instance to greyscale. 
	/// </summary>
	void ConvertPixelArrayToGreyscale() 
	{
		BYTE greyValue = 0;

		for (size_t i = 0; i < pixelArray.get_rows(); ++i) {
			for (size_t j = 0; j < pixelArray.get_cols(); ++j)
			{
				greyValue = static_cast<BYTE>(	0.299 * static_cast<double>(pixelArray[i][j].rgbRed) + \
												0.587 * static_cast<double>(pixelArray[i][j].rgbGreen) + \
												0.114 * static_cast<double>(pixelArray[i][j].rgbBlue));

				pixelArray[i][j].rgbRed		= greyValue;
				pixelArray[i][j].rgbGreen	= greyValue;
				pixelArray[i][j].rgbBlue	= greyValue;
			}
		}
	}

	/// <summary>
	/// Conversion of pixelArray member of ImageWrapper instance to greyscale. The output / input argument will be created with the same size as pixelArray. Its RGB values will all be filled with the corresponding greyscale value
	/// </summary>
	/// <param name="outGreyscaleArray"></param>
	void GetPixelArrayAsGreyscale(Array2D<RGBQUAD>& outGreyscaleArray) {

		BYTE greyValue = 0;

		outGreyscaleArray.create(pixelArray.get_rows(), pixelArray.get_cols());

		for (size_t i = 0; i < outGreyscaleArray.get_rows(); ++i) {
			for (size_t j = 0; j < outGreyscaleArray.get_cols(); ++j)
			{
				greyValue = static_cast<BYTE>(	0.299 * static_cast<double>(pixelArray[i][j].rgbRed) + \
												0.587 * static_cast<double>(pixelArray[i][j].rgbGreen) + \
												0.114 * static_cast<double>(pixelArray[i][j].rgbBlue));

				outGreyscaleArray[i][j].rgbRed		= greyValue;
				outGreyscaleArray[i][j].rgbGreen	= greyValue;
				outGreyscaleArray[i][j].rgbBlue		= greyValue;
			}
		}
	}


	/// <summary>
	/// In place conversion of RGBQUAD to Greyscale RGBQUAD which converts the RGB Colors to the same greyscale value
	/// </summary>
	/// <param name="colorInOut"></param>
	static inline void RGBQUADToGreyscale(RGBQUAD& colorInOut) 
	{
		BYTE greyValue = static_cast<BYTE>(	0.299 * static_cast<double>(colorInOut.rgbRed) + \
											0.587 * static_cast<double>(colorInOut.rgbGreen) + \
											0.114 * static_cast<double>(colorInOut.rgbBlue));

		colorInOut.rgbRed	= greyValue;
		colorInOut.rgbGreen	= greyValue;
		colorInOut.rgbBlue	= greyValue;
	}

	/// <summary>
	/// Converts RGB Value of RGBQuad to a single greyscale value of type double
	/// </summary>
	/// <param name="colorIn"></param>
	/// <returns>Greyscale Value (double)</returns>
	static inline double RGBQUADToGreyscaleDouble(RGBQUAD& colorIn)
	{
		return static_cast<double>(	0.299 * static_cast<double>(colorIn.rgbRed) + \
									0.587 * static_cast<double>(colorIn.rgbGreen) + \
									0.114 * static_cast<double>(colorIn.rgbBlue));
	}

	/// <summary>
	/// Converts RGB Value of RGBQuad to a single greyscale value of type BYTE
	/// </summary>
	/// <param name="colorIn"></param>
	/// <returns>Greyscale Value (BYTE)</returns>
	static inline BYTE RGBQUADToGreyscaleBYTE(RGBQUAD& colorIn)
	{
		return static_cast<BYTE>(0.299 * static_cast<double>(colorIn.rgbRed) + \
								 0.587 * static_cast<double>(colorIn.rgbGreen) + \
								 0.114 * static_cast<double>(colorIn.rgbBlue));
	}

};

