
Notes:

- The x64 target is not supported yet (it's started being worked on but needs to be finished)

Setup:

- The config.inc file contains a flag to support the SSE. Setting it to 0 will force it to use the FPU instead.
- The defines from compile.inc should generally NOT be touched.

To compile:

- In the build.bat specify the path to ml.exe (should be in the Visual Studio folders)
- Use ML from VS2003 if possible (I remember there were some issues with some other versions)
