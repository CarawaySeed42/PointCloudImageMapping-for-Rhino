# -*- coding: utf-8 -*-
import rhinoscriptsyntax as rs
import Rhino
import System.Drawing.Color
import math
from scriptcontext import doc

def createBrandingGeometry():
    
    center_point = rs.GetPoint("Please choose center of the image!")
    
    if not center_point:
        return
    # Color to use when drawing dynamic lines
    line_color_red = System.Drawing.Color.FromArgb(255,0,0)
    line_color_green = System.Drawing.Color.FromArgb(0,255,0)
    line_color_black = System.Drawing.Color.FromArgb(0,0,0)
    
    draw_Color = line_color_green
    
    # This is a function that is called whenever the GetPoint's
    # DynamicDraw event occurs
    def GetPointDynamicDrawFunc( sender, args ):
        currentPoint = args.CurrentPoint
        #draw a line from the first picked point to the current mouse point
        args.Display.DrawLine(center_point, currentPoint, draw_Color, 1)
    # Create an instance of a GetPoint class and add a delegate
    # for the DynamicDraw event
    gpRight = Rhino.Input.Custom.GetPoint()
    gpRight.DynamicDraw += GetPointDynamicDrawFunc
    gpRight.SetCommandPrompt("Please chose direction to the right of the image!")
    gpRight.Get()
    if( gpRight.CommandResult() == Rhino.Commands.Result.Success ):
        pt = gpRight.Point()
        rightLine = rs.AddLine(center_point, pt)
        rs.ObjectColor(rightLine, [0,255,0])
    else:
        return
        
    draw_Color = line_color_red
    gpUp = Rhino.Input.Custom.GetPoint()
    gpUp.DynamicDraw += GetPointDynamicDrawFunc
    gpUp.SetCommandPrompt("Please chose up direction of the image!")
    gpUp.Get()
    if( gpUp.CommandResult() == Rhino.Commands.Result.Success ):
        pt = gpUp.Point()
        line_up = Rhino.Geometry.Line(center_point, pt)
        line_right = rs.coerceline(rightLine)
        normal = Rhino.Geometry.Vector3d.CrossProduct(line_right.Direction, line_up.Direction)
        angle = math.radians(90)
        xform = Rhino.Geometry.Transform.Rotation(angle, normal, center_point)
        line_right.Transform(xform)
        upLine = rs.AddLine(line_right[0], line_right[1])
        rs.ObjectColor(upLine, [255,0,0])
    else:
        rs.DeleteObject(rightLine)
        return
    
    # Create Final Geometry
    line_right = rs.coerceline(rightLine)
    line_up = rs.coerceline(upLine)
    
    rs.DeleteObject(rightLine)
    rs.DeleteObject(upLine)
    finalGuid = rs.AddPolyline([line_right.To, line_right.From, line_up.To])
    rs.ObjectColor(finalGuid, [0,255,0])
    
    if not rs.IsLayer("IMAGE_MAP_GEOMETRIES"):
        f_layer = rs.AddLayer("IMAGE_MAP_GEOMETRIES")
    else:
        f_layer = "IMAGE_MAP_GEOMETRIES"
    rs.ObjectLayer(finalGuid, f_layer)
    
    rs.SetUserText(finalGuid, "Image Mapping Geometry", True, attach_to_geometry=False)
    
    # Being the geometry to front for better visibility
    obj = doc.Objects.Find(finalGuid)
    rs.UnselectAllObjects()
    rs.SelectObjects(finalGuid)
    patch = rs.Command("-_BringToFront _enter", echo = False)
    rs.UnselectObjects(finalGuid)
    #obj.Attributes.DisplayOrder = 1


if __name__ == "__main__":
    createBrandingGeometry()
