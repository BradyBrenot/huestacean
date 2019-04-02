Huestacean is a Philips Hue screen syncing app for Desktop and Android devices. It uses Philips' new Entertainment API to sync the user's screen with their lights with very low latency.

<a href="http://www.youtube.com/watch?feature=player_embedded&v=sPH1u7Z7y9E" target="_blank"><img src="https://i.imgur.com/hICos2X.png" alt="Youtube video demonstrating Huestacean's screen sync feature" border="0" width="350px" /></a>
<img src="https://i.imgur.com/H67w2ta.png" alt="Youtube video demonstrating Huestacean's screen sync feature" border="0" width="250px" />


## Download
Downloadable binaries for Windows, macOS, and Android are available on the **[Releases](https://github.com/BradyBrenot/huestacean/releases)** page. It ought to also be possible to build the source for Linux but this has not yet been tested, please read the [README](https://github.com/BradyBrenot/huestacean/blob/master/README.md) for instructions on building from source.

## Safety notice
This software can cause rapid flickering of your Hue lights that may trigger photosensitive epileptic seizures in vulnerable individuals. ***DO NOT*** use this if you are or suspect you are affected by photosensitive epilepsy, or are otherwise photosensitive, epileptic, or suffer from seizures generally. Cease use immediately and consult a doctor if you suffer from any seizure symptoms, which may include lightheadedness, altered vision, eye or face twitching, jerking or shaking of arms or legs, disorientation, confusion, or momentary loss of awareness.

## General notes

### Hue requirements
A Gen2 bridge is a **must**. Your bridge and lights should also be using the latest firmware. Use the Philips Hue Android or iOS app to update the firmware.

At the moment this software can't create entertainment groups. You need to do this in the Hue app. Philips has a video describing how to do this [on the Hue Youtube channel](https://www.youtube.com/watch?v=_N7VNJM_8js).

### Lights
The Lightstrip Plus is by far the best light I've found for this. It has a wide color gamut, and it dims to a *super dark* state before it turns off completely. My Gen 1 lights are still pretty bright at their dimmest. This can be a problem when syncing the lights in a darkened room.

### Positioning lights
For the best experience, I'd suggest using or more lights behind or to the side of the display, in front of you and within the central cone of vision. Rapid changes in lights behind or to your side are likely to be more *distracting* and *annoying* than they are actually enjoyable. The video above is more lights than I actually use this with; usually I only have the one lightstrip behind my TV syncing.

## Windows
Only (64-bit) Windows 8 and 10 are fully supported as the application uses the [IDXGIOutputDuplication](https://msdn.microsoft.com/en-us/library/windows/desktop/hh404611(v=vs.85).aspx) API which is only available from Windows 8 onwards. This allows the application to capture almost anything, including fullscreen games, with extremely high performance. If someone else is interested in working on Windows 7 support I'd welcome PRs, though.

If you have a Hybrid GPU setup (some laptops, convertibles, etc.), make sure you run Huestacean on the **Integrated GPU**, not the Discrete GPU.

### Prerequisites
* [Visual Studio 2017 redistributable](https://aka.ms/vs/15/release/VC_redist.x64.exe)

### Install
Download the latest from [releases](https://github.com/BradyBrenot/huestacean/releases). Extract anywhere. Run `huestacean.exe`

## macOS
Only supported on macOS 10.7+ (uses [AVCaptureScreenInput](https://developer.apple.com/documentation/avfoundation/avcapturescreeninput)). Performance may be worse than Windows and Android.

Download the .app from [releases](https://github.com/BradyBrenot/huestacean/releases) and run it.

So far it's only been tested on 10.11.6 on an old Macbook Pro.

## Linux
Build from source. See the [README](https://github.com/BradyBrenot/huestacean/blob/master/README.md) for more information. Performance may be worse than other platforms.

## Android
This is still **experimental**, and is known to crash. Side-load the APK and run it. It does not currently run as a service so Android will kill it to save resources if it thinks it needs to. Requires Android 5.0 (uses [MediaProjection](https://developer.android.com/reference/android/media/projection/package-summary.html))

The apk should work for most modern **ARM**-based Android devices with at least **Android 5**

 - Has not been tested on any real hardware older than Android 7.0, although it has been tested on 5.1 on an emulator.
 - [Turn off battery optimization](https://www.google.ca/search?q=android+turn+off+battery+optimization+for+app) on the app or Android will eventually suspend it while it's in the background!
 - Works just great on my old Galaxy Note 3 (with Lineage OS), which is more than four years old; I can streami Youtube and you couldn't tell the difference from when I run it on my PC.
 - Will **not** work in Netflix and other video apps that tell Android they're showing "secure" content.
   - Xposed's DisableSecureFlag module can bypass this but _don't install that_ unless you actually know what you're doing and how to recover from a boot loop. You could hose your device, do this **at your own risk**. Also mind that this allows other screen recording software to record other "secure" apps like banking apps.

## Reporting bugs
Use the repository's [Issues](https://github.com/BradyBrenot/huestacean/issues) to report bugs or other problems.

## External libraries
This project is using:
- [Qt](https://www.qt.io/)
- [mbedtls](https://github.com/ARMmbed/mbedtls)
- [screen_capture_lite](https://github.com/smasherprog/screen_capture_lite)

## License

Refer to the [LICENSE](LICENSE) file for license info.

### Third-party licenses

This software makes use of open source software under various licenses, which may be found in the **third_party_licenses** directory.

* Qt: lgpl-3.txt, gpl-3.txt, Qt_ThirdPartySoftware_Listing.txt
* screen_capture_lite: license-screen_capture_lite.txt
* mbedtls and grafika: apache-2.0.txt

This software uses Qt 5, which is licensed under the GNU Lesser General Public License v3.0. The text of the license can be found in **third_party_licenses/lgpl-3.txt**. The corresponding source code for Qt can be found [on their website](https://www.qt.io/download), or at https://s3.us-east-2.amazonaws.com/bbrenot-thirdparty-sourcecode/qt-everywhere-src-5.10.0.tar.xz ; the end user can provide their own Qt5 and modifications by replacing the distributed Qt shared library files (Qt* .dll, .so, .dylib, etc.)
