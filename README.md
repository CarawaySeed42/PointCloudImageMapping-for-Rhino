# PointCloudImageMapping-for-Rhino
This is a small project that allows a user to project images onto their point clouds within the CAD Software Rhinoceros 3D (Rhino)<br>
Due to this project being just for fun, there was no emphasis on clean code

## Contents
- Code source for a C++ DLL that provides a fast image mapping routine
- Python scripts to easily call said image mapping routine
- Python modules for input dialogs and library loading

## How to use
- Load a Point cloud object to Rhino workspace
- Run "...\python\main\createImageMapGeometry.py"
- Place the geometry where you want your image to be projected to
- Run "...\python\main\mapImageToPointCloud.py"
- Choose your mapping geometry and your point cloud
- Select a image from your file system (e.g. image.png)
- Choose from different mapping settings (Colored, Greyscale or raise cloud by value)
- Enter a image width and the depth of the mapped image
- Done

## How to build
Note:
- This code was developed on and for the x64 platform on Windows 10

General Build Requirements:
- Use the corresponding solution and build with Visual Studio 2019 or Visual Studio 2022
- Needs Rhino 7 SDK installed and linked (C:\Program Files\Rhino 7 SDK\PropertySheets)
- Needs FreeImage Library header, dll, lib in corresponding folder (Please see "How to get FreeImageLibrary" below)

Build:
- If requirements are met then run "Build -> Rebuild Solution" or "Build -> Build Solution"

Additional VS Components:
- Needs Visual Studio SDK 
- Needs "C++-MFC for latest vXXX-Buildtools (x86 and x64)" (Where XXX is the platform toolset version like 142 (VS2019) or 143(VS2022))

How to get FreeImageLibrary:
- [Download FreeImage DLL](https://freeimage.sourceforge.io/download.html)
- Extract Contents
- Copy or move extracted .h, .dll and .lib files for your platform to FreeImage folder
  Example: Move Contents of "...\FreeImage3180Win32Win64\FreeImage\Dist\x64\" to "...\PointCloudImageMapping\cpp\FreeImg\"

Additional Information:
- For necessary Tools to code for Rhino on Windows please [see here](https://developer.rhino3d.com/guides/cpp/installing-tools-windows/)
- For a step by step manual how to setup a visual studio solution like this [see here](https://discourse.mcneel.com/t/step-by-step-example-for-adding-c-dll-to-a-python-script/126074)<br>
Thanks to Terry_Chappell for this great guide with examples

## Example

![Unmapped_image](https://user-images.githubusercontent.com/112782924/213671459-f07e4974-8cb5-4918-a879-1186568ba4f5.png)<br>
![Mapped_Image](https://user-images.githubusercontent.com/112782924/213671214-38c89a56-ae2c-4ee1-9fe0-05b7e17267c3.png)

Top: Raw Point Cloud with target geometry<br>Bottom: Mapping a free wallpaper to the point cloud with target geometry still visible in the middle
