git submodule update --recursive
mkdir build
mkdir build\debug
mkdir build\release
cd build
SET CMAKE_PREFIX_PATH=C:\Qt\5.10.0\msvc2017_64\lib\cmake
cmake .. -G "Visual Studio 15 2017 Win64"
cd debug
msbuild ../Huestacean.vcxproj /property:Configuration=Debug /property:Platform=x64
cd ../release
msbuild ../Huestacean.vcxproj /property:Configuration=Release /property:Platform=x64