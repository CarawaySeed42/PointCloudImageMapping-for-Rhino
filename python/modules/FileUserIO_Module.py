#coding: utf-8
import Rhino as rc
import rhinoscriptsyntax as rs
import System
import Rhino.UI
import Eto.Drawing as drawing
import Eto.Forms as forms

    
class InputDialog_LabelBox(forms.Dialog[bool]):
    
    def __init__(self, labelList, defaultInputList, dialogTitle = ""):
        """
        __init__(self, labelList, defaultInputList, dialogTitle = "")
        This is a first custom class for Text Input Dialogs
        It will create a EtoInputDialogForm with as many labels plus textboxes as there are in the initializer lists
        So four labels plus four default Inputs will create four textboxes
        
        Constructor arguments:
            labelList (strings)         : Labels/Descriptions of the text boxes
            defaultInputList (strings)  : Default text inside the textboxes that can be manipulated by the user
            dialogTitle (opts string)   : Title of the dialog form
        """
        labelCount = len(labelList)
        inputCount = len(defaultInputList)
        
        if labelCount != inputCount:
            raise Exception("EtoInputDialog_LabelBox_3DM: Label List and Default Value List have different sizes! This is not allowed!")
        
        # Initialize dialog box
        self.Title = dialogTitle
        self.Padding = drawing.Padding(35)
        self.Resizable = False
        
        self.m_count = labelCount
        self.m_labels = labelList
        self.m_textInputs = defaultInputList
        
        # Content Row Start has to be different to 0 if later on there will be rows added above the input
        self.contentRowStart = 0
        
        # Create the default button
        self.DefaultButton = forms.Button(Text = 'OK')
        self.DefaultButton.Click += self.OnOKButtonClick
 
        # Create the abort button
        self.AbortButton = forms.Button(Text = 'Cancel')
        self.AbortButton.Click += self.OnCloseButtonClick
        
        # Create a table layout and add all the controls
        layout = forms.DynamicLayout()
        layout.Spacing = drawing.Size(10, 10)
        
        # Create controls for the dialog
        for label, input in zip(labelList, defaultInputList):
            layout.AddRow(forms.Label(Text = label), forms.TextBox(Text = input))
        
        layout.AddRow(" ") # spacer
        layout.AddRow(self.DefaultButton, self.AbortButton)
        
        # Set the dialog content
        self.Content = layout
 
    # Start of the class functions
 
    # Get the value of the textboxes
    def GetTextBoxInputs(self):
        """
        GetTextBoxInputs()
        Returns list of values in the dialogs text boxes
        """
        outputList = [None] * self.m_count
        rowTable = self.Content.Rows
        
        for idx in range(0, self.m_count):
            # Offset the row of the textbox by contentRowStart. Again: If content doesn't start in first row then an offset has to be added
            contentIDX = self.contentRowStart + idx 
            outputList[idx] = rowTable[contentIDX][1].Control.Text
            
        return outputList
    
    # Close button click handler
    def OnCloseButtonClick(self, sender, e):
        self.Close(False)
    
    # OK button click handler
    def OnOKButtonClick(self, sender, e):
        self.Close(True)
        
    # Show Dialog Window to the user
    def ShowDialog(self):
        """
        ShowDialog()
        Show Dialog to let user input whatever there is to ask for
        """
        return self.ShowModal(Rhino.UI.RhinoEtoApp.MainWindow)
        
   

def OpenFileDiag(title=None, filter=None, folder=None, multiSelect = True):
    """
    OpenFileDiag(title=None, filter=None, folder=None, multiSelect = True)
    Shows OpenFileDialog for file selection.
    This is basically just a wrapper for System.Windows.Forms.OpenFileDialog()
    
    Input:
        title (string)      : Title of the dialog
        filter (string)     : File extensions to filter by
        folder (string)     : Initial directory of the dialog
        multiSelect (bool)  : Is it allowed to select multiple files
    
    Returns:
        result (list or string) : Path to selected file or files as string or list
    """
    fd = System.Windows.Forms.OpenFileDialog()
    
    if folder == " ":
        folder = None
    
    fd.InitialDirectory = r"{}".format(folder)
        
    fd.Title = r"{}".format(title)
    fd.Filter = filter
    fd.FilterIndex = 1
    fd.RestoreDirectory = True
    fd.Multiselect = multiSelect
    
    if fd.ShowDialog() == System.Windows.Forms.DialogResult.OK:
        result = fd.FileNames
        return result
    else:
        return []
