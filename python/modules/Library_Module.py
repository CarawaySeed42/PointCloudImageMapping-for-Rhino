#coding: utf-8
# Module used to load and unload pcloud mapping lib

def load_CloudImageMap_Lib():
    """
    load_CloudImageMap_Lib()
    Loads the C++ PointCloudImageMap Library. Returns object with lib handle to call library functions 
    """
    import os
    from ctypes import cdll
    
    cwd = os.getcwd()
    __parentsParentDirectory = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    branding_dll_name = os.path.join(__parentsParentDirectory, 'bin\PointCloudImageMapping.dll') 
    os.chdir(os.path.dirname(branding_dll_name))
    
    # Try to load DLL.
    try:
        branding_dll = cdll.LoadLibrary(r"{}".format(branding_dll_name))
        os.chdir(cwd)
        return branding_dll
        
    except Exception as ex:
        os.chdir(cwd)
        raise Exception('\n{}\n\nCould not load Point Cloud Image Mapping Lib:\n{}'.format(ex, branding_dll_name))


def UnloadLibrary(loaded_dll, forceUnload = False):
    """
    UnloadLibrary(loaded_dll, forceUnload = False)
    
    Unloads a shared library
    loaded_dll:             dll object with handle
    forceUnload (opts):     If True then unload as many times as necessary to completely remove all references
                            If False then trigger unload once. If it was loaded more than once then the dll is still referenced
    
    Returns True if library was loaded before unloading, False if not and should always return false if unloading is forced
    """
    import sys
    import ctypes
    import platform
    
    OS = platform.system()
    
    if OS == "Windows":  # pragma: Windows
        dll_close = ctypes.windll.kernel32.FreeLibrary
    
    elif OS == "Darwin":
        try:
            try:
                # macOS 11 (Big Sur). Possibly also later macOS 10s.
                stdlib = ctypes.CDLL("libc.dylib")
            except OSError:
                stdlib = ctypes.CDLL("libSystem")
        except OSError:
            # Older macOSs. Not only is the name inconsistent but it's
            # not even in PATH.
            stdlib = ctypes.CDLL("/usr/lib/system/libsystem_c.dylib")
        dll_close = stdlib.dlclose
    
    elif OS == "Linux":
        try:
            stdlib = ctypes.CDLL("")
        except OSError:
            # Alpine Linux.
            stdlib = ctypes.CDLL("libc.so")
        dll_close = stdlib.dlclose
    
    elif sys.platform == "msys":
        # msys can also use `ctypes.CDLL("kernel32.dll").FreeLibrary()`. Not sure
        # if or what the difference is.
        stdlib = ctypes.CDLL("msys-2.0.dll")
        dll_close = stdlib.dlclose
    
    elif sys.platform == "cygwin":
        stdlib = ctypes.CDLL("cygwin1.dll")
        dll_close = stdlib.dlclose
    
    elif OS == "FreeBSD":
        # FreeBSD uses `/usr/lib/libc.so.7` where `7` is another version number.
        # It is not in PATH but using its name instead of its path is somehow the
        # only way to open it. The name must include the .so.7 suffix.
        stdlib = ctypes.CDLL("libc.so.7")
        dll_close = stdlib.close
    
    else:
        raise NotImplementedError("    Could not unload library. Unknown platform!")
    
    dll_close.argtypes = [ctypes.c_void_p]
    wasLoaded = dll_close(loaded_dll._handle)
    
    # If forceUnload then unload until unloading fails because there is nothing to unload anymore
    if forceUnload:
        while wasLoaded:
            wasLoaded = dll_close(loaded_dll._handle)
    
    return wasLoaded
