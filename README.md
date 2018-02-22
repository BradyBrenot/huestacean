# Huestacean

Philips Hue screen syncing app for Desktop. Uses Philips' new Entertainment API to sync the user's screen with their lights with very low latency.

<a href="http://www.youtube.com/watch?feature=player_embedded&v=sPH1u7Z7y9E" target="_blank"><img src="https://i.imgur.com/hICos2X.png" alt="Youtube video demonstrating Huestacean's screen sync feature" border="0" width="350px" /></a>
<img src="https://i.imgur.com/H67w2ta.png" alt="Youtube video demonstrating Huestacean's screen sync feature" border="0" width="250px" />

## Safety notice
This software can cause rapid flickering of your Hue lights that may trigger photosensitive epileptic seizures in vulnerable individuals. ***DO NOT*** use this if you are or suspect you are affected by photosensitive epilepsy, or are otherwise photosensitive, epileptic, or suffer from seizures generally. Cease use immediately and consult a doctor if you suffer from any seizure symptoms, which may include lightheadedness, altered vision, eye or face twitching, jerking or shaking of arms or legs, disorientation, confusion, or momentary loss of awareness.

## General notes
Binaries are currently available only for Windows (see [Installing](#installing)). Binaries for macOS are on the TODO list. It ought to also be possible to build the source for Linux, see ***Building*** below.

### Hue requirements
A Gen2 bridge is a **must**. Your bridge and lights should also be using the latest firmware. Use the Philips Hue Android or iOS app to update the firmware.

At the moment this software can't create entertainment groups. You need to do this in the Hue app. Philips has a video describing how to do this [on the Hue Youtube channel](https://www.youtube.com/watch?v=_N7VNJM_8js).

### Lights
The Lightstrip Plus is by far the best light I've found for this. It has a wide color gamut, and it dims to a *super dark* state before it turns off completely. My Gen 1 lights are still pretty bright at their dimmest. This can be a problem when syncing the lights in a darkened room.

### Positioning lights
For the best experience, I'd suggest using or more lights behind or to the side of the display, in front of you and within the central cone of vision. Rapid changes in lights behind or to your side are likely to be more *distracting* and *annoying* than they are actually enjoyable.

## Installing
### Windows
Only 64-bit Windows is supported. Only Windows 10 has been tested at this point.

#### Prerequisites
* [Visual Studio 2017 redistributable](https://aka.ms/vs/15/release/VC_redist.x64.exe)

#### Install
Download the latest from [releases](https://github.com/BradyBrenot/huestacean/releases). Extract anywhere. Run `huestacean.exe`

## Building
### Dependencies
* Qt 5.10
* CMake 3.11

#### Windows
* Visual Studio 2017. Community Edition is fine.

### All platforms
Clone the repository and its submodules
```
git clone --recursive git://github.com/BradyBrenot/huestacean.git
cd huestacean
```

### Windows
Run the 'x64 Native Tools Command Prompt for VS 2017'. `cd` to the repository directory.

Assuming you have Qt5.10 installed in `C:\Qt\5.10.0`, run:

```
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
```
Then use [windeployqt](http://doc.qt.io/qt-5/windows-deployment.html) to copy in the necessary deployment files.

### Linux, OS X
Are as-yet untested
```
mkdir build
cd build
cmake ..
make
```

## Reporting bugs
Use this repository's [Issues](https://github.com/BradyBrenot/huestacean/issues) to report bugs or other problems.

## External libraries
This project is using:
- [Qt](https://www.qt.io/)
- [mbedtls](https://github.com/ARMmbed/mbedtls)
- [screen_capture_lite](https://github.com/smasherprog/screen_capture_lite)

## License

Refer to the [LICENSE](LICENSE) file for license info.

### Third-party licenses

This software makes use of open source software under various licenses. See [qml/about.qml](qml/about.qml) for a full listing of licenses both applicable and not, or the various text files included with the source distribution.

This software uses Qt 5, which is licensed under the GNU Lesser General Public License v3.0. The text of the license can be found in [lgpl-3.txt](lgpl-3.txt). The corresponding source code for Qt can be found [on their website](https://www.qt.io/download), or at https://s3.us-east-2.amazonaws.com/bbrenot-thirdparty-sourcecode/qt-everywhere-src-5.10.0.tar.xz ; the end user can provide their own Qt5 and modifications by replacing the distributed Qt shared library files (Qt* .dll, .so, .dylib, etc.)
