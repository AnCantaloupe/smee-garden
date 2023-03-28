# About
This program formed part of the marketing that SMEE did for our Garden Sale & Tea event. It was run on my laptop, which was hooked up to the TV pointing out of the SMEE room, and people in the corridor could interact with it using a light-based controller.

# Build
I only tested this on Windows using MSVC; I don't think there's anything in there that's specific to MSVC, but I can't promise that it will work well with other compilers.
Note that you will need to have MSVC installed prior to following these steps.
1. Open a command terminal
2. Navigate to the directory **smee-garden/src**
3. Run the command ***setup_cl_generic x64*** or ***setup_cl_generic x86***
	This will run the script *vcvarsall.bat* in the Visual Studio install folder, setting things up to allow us to invoke the compiler
4. Run the command ***build.bat***
	This will create the directory **smee-garden/build** containing the executable **windows.exe**