# -*- coding: utf-8 -*-

import rhinoscriptsyntax as rs
import os, sys
import Rhino as rc
from Rhino.Geometry import PointCloud
from scriptcontext import doc
import scriptcontext
from ctypes import c_bool, c_float, byref, cdll, c_double, c_char_p, wintypes
from Rhino.Runtime import Interop
import System.Windows.Forms as winForms
import traceback

__parentDirectory = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
__module_path = os.path.join(__parentDirectory, 'modules') 
sys.path.append(r"{}".format(__module_path))

def __moveImageNumber(curNum, MaxNum):
    
    newNum = curNum + 1
    if newNum > MaxNum:
        newNum = 0
    return newNum

def ListBoxBranding():
    
    optionPicked = rs.ListBox( ['Color Only', 'Greyscale Only', 'Elevate Only', 'Color + Elevate', 'Greyscale + Elevate'], "Please choose a Setting!")
            
    if not optionPicked: 
        raise Exception("No Options chosen!")
    
    if optionPicked == "Color Only":
        return 0
    elif optionPicked == "Greyscale Only":
        return 1
    elif optionPicked == "Elevate Only":
        return 2
    elif optionPicked == "Color + Elevate":
        return 3
    elif optionPicked == "Greyscale + Elevate":
        return 4
    else:
        return 1

def pointCloudImageMap():
    pcImgMapLib = LibLoad.load_CloudImageMap_Lib()
    
    poly_objects = rs.GetObjects("Please Select Image Mapping Geometry(s)!", 4, True, True)
    if not poly_objects: raise Exception("No Image Mapping Geometry selected!")
    if not all([rs.IsPolyline(curve) or rs.IsLine(curve) for curve in poly_objects]):
        rs.MessageBox("Only Polylines are supported!")
        return
    
    pc_objects = rs.GetObjects("Please Select Point Cloud(s)!", 2, True, True)
    if not pc_objects: raise Exception("No Point Cloud Selected!")
    
    files = IO.OpenFileDiag("Please choose an image!","Image Files|*.png;*.jpg;*.jpeg;*.bmp;*.ico;*.tiff;*.tif|All files (*.*)|*.*")
    if not files: return
    imageCount = len(files)
    curImageNum = 0
    
    # Let User choose option
    optionNumber = ListBoxBranding()
    
    # Initialize values
    imageWidth      = 4
    imageDepth      = 0.5
    raiseValue      = 0.02
    
    #Set colorMapping, greyscale and raise from chosen option
    colorMapping    = (optionNumber in [0,1,3,4])
    greyscale       = (optionNumber in [1,4])
    raised          = (optionNumber in [2,3,4])
    
    # Set Up image parameters
    descriptions = ['Image Width [m]', 'Image Depth [m]', 'White Elevation (Raise) [m]']
    dialogValues = ['4', '0.2', '0.02']
    
    # Limit options and omit raiseValue if no elevation option chosen
    valueSlice = len(dialogValues)
    if optionNumber < 2:
        valueSlice -= 1
    
    dialog = IO.InputDialog_LabelBox(descriptions[0:valueSlice], dialogValues[0:valueSlice], "Please Enter Parameters")
    dialogRC = dialog.ShowDialog()
    
    if not dialogRC: return
    else:
        props = dialog.GetTextBoxInputs()
        imageWidth = float(props[0])
        imageDepth = float(props[1])
        if len(props) >=3:
            raiseValue = float(props[2])
    
    for cloudIdx,cloud in enumerate(pc_objects):
        cloudGeo = rs.coercegeometry(cloud)
        
        for polyIdx,poly in enumerate(poly_objects):
            rs.Prompt("Projecting image at Geometry {} to cloud {} ...".format(polyIdx+1, cloudIdx+1))
            
            if not (rs.IsPolyline(curve) or rs.IsLine(curve)):
                print("Image Mapping Geometry is no Line or Polyline!")
                continue
            
            imagePath = files[curImageNum]
            curImageNum = __moveImageNumber(curImageNum, imageCount-1)
            
            scriptcontext.escape_test()
            rc.RhinoApp.Wait()
            
            isBrandingGeo = rs.GetUserText(poly, "Image Mapping Geometry")
            if not isBrandingGeo:
                print("Geometry is no Image Mapping Geometry!")
                continue
            
            #CTypes
            pcPointer = Interop.NativeGeometryNonConstPointer(cloudGeo)
            polyGeo = rs.coercegeometry(poly)
            polyPointer = Interop.NativeGeometryNonConstPointer(polyGeo)
            
            # Create C-types variable to hold data. Initialize with Python data.
            c_FilePath = wintypes.LPCWSTR(r"{}".format(imagePath))
            c_imageWidth = c_double(imageWidth)
            c_imageDepth = c_double(imageDepth)
            c_colorMapping = c_bool(colorMapping)
            c_greyscale = c_bool(greyscale)
            c_raised = c_bool(raised)
            c_raiseValue = c_double(raiseValue)
            ctd = (c_float)()
            
            c_success = c_bool(False)
            
            c_success = pcImgMapLib.PointCloudImageMapping(pcPointer, polyPointer, c_FilePath, c_imageWidth, c_imageDepth, c_colorMapping, c_greyscale, \
            c_raised, c_raiseValue, byref(ctd))
            
            if not c_success:
                continue
                
 # bool PointCloud_Branding(ON_PointCloud* inputCloudPointer, ON_Curve* orientPoly, LPCWSTR fileWideString, double width, double depth = 0.2, bool colorMapping = true,\
 #                          bool greyscale = false, bool raised = false, double raiseValue = 0.02, uint32_t maxThreadCount = 4, float& td = defaultArg)
            print 'Time to brand Cloud = {0:.4f} sec'.format(ctd.value)
            scriptcontext.escape_test()
            scriptcontext.doc.Objects.Replace(cloud, cloudGeo)
            rc.RhinoApp.Wait()
            
        cloudGeo.Dispose()
    LibLoad.UnloadLibrary(pcImgMapLib, True)
    rs.Redraw()

if __name__ == "__main__":
    import FileUserIO_Module as IO
    import Library_Module as LibLoad
    
    try:
        pointCloudImageMap()
    except Exception as ex:
        traceback.print_exc()
