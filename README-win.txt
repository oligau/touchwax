1. Install Microsoft Visual Studio Express 2012
2. Compile pthread-win32 as static library
# C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat
# cd touchwax\pthread-win32
# nmake clean VC-static
3. Compile SDL2 static library
  - Open touchwax\SDL2\VisualC\SDL_VS2012EE.sln in VC Express 2012
  - Right click on sdl project in Solution Explorer, choose Build.
  - Right click on sdl_main project in Solution Explorer, choose Build.
3. Compile liblo static library
  - Open touchwax\lo\build\vs2012\liblo.vcxproj in VC Express 2012
  - Right click on liblo project in Solution Explorer, choose Build.
4. Open touchwax\touchwax\touchwax_VC2012.vcxproj in VC Express 2012
  - Right click on touchwax project in Solution Explorer, choose Build.
5.