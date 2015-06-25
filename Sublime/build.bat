@echo off

:: // TODO: find a way to make this unnecessary 
:: subst w: ..

:: for release builds
:: MSBuild ..\\GLMario\\GLMario.vcxproj /p:Configuration=Release /p:OutDir="..\\Release\\"
::start w:\Release\\OpenGL_Base.exe

:: for debug builds
MSBuild ..\\GLMario\\GLMario.vcxproj /p:Configuration=Debug /p:OutDir="..\\Debug\\"

::start w:\Debug\\OpenGL_Base.exe :: Potentially not working due to running as it is compiling? but weird, because the window size is correct, and it opens.

::Flags
:: /p:Configuration=Release  // release build

:: /p:OutputPath=c:\mydir 	// set output directory