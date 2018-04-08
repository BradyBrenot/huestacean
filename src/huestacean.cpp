#include "huestacean.h"
#include "entertainment.h"
#include "utility.h"

#include <algorithm>
#include <cmath>

#include <QQmlApplicationEngine>
#include <QElapsedTimer>
#include <QTimer>
#include <QSettings>
#include <QKeyEvent>
#include <QCoreApplication>

#if ANDROID
#include <QAndroidJniObject>
#include <GLES2/gl2.h>
#endif

#if __APPLE__
#include "mac/screencapture.h"
#endif

QNetworkAccessManager* qnam = nullptr;
QMutex huestaceanLock;
Huestacean* huestaceanInstance = nullptr;

Huestacean::Huestacean(QObject *parent) 
    : QObject(parent),
    syncing(false)
{
    if(qnam == nullptr)
        qnam = new QNetworkAccessManager(nullptr);

    bridgeDiscovery = new BridgeDiscovery(this);
    bridgeDiscovery->startSearch();
    detectMonitors();

    connect(bridgeDiscovery, SIGNAL(modelChanged()),
        this, SLOT(connectBridges()));

    eImageProvider = new ScreenSyncImageProvider(this);
    extern QQmlApplicationEngine* engine;
    engine->addImageProvider("entimage", eImageProvider);

    emit hueInit();

	ReadSettings();

    //ENTERTAINMENT
	streamingGroup = nullptr;

#if !ANDROID && (defined(__linux__) || defined(Q_OS_LINUX))
	mipMapGenerationEnabled = false;
#else
    mipMapGenerationEnabled = true;
#endif

    qmlRegisterType<EntertainmentGroup>();

    huestaceanLock.lock();
    huestaceanInstance = this;
    huestaceanLock.unlock();
    
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SIGNAL(frameReadElapsedChanged()));
    timer->start(500);
}

Huestacean::~Huestacean()
{
	WriteSettings();

    huestaceanLock.lock();
    if(huestaceanInstance == this)
    {
        huestaceanInstance = nullptr;
    }
    huestaceanLock.unlock();

    for (QObject* Obj : bridgeDiscovery->getModel())
    {
        HueBridge* bridge = qobject_cast<HueBridge*>(Obj);
        if (bridge != nullptr)
        {
            for (auto& group : bridge->EntertainmentGroups)
            {
                group->shutDownImmediately();
                while (group->hasRunningThread())
                {
                    QThread::msleep(100);
                }
            }
        }
    }
    
#if __APPLE__
    macScreenCapture::stop();
#endif
}

void Huestacean::ReadSettings()
{
	QSettings settings;
	settings.beginGroup("entertainment");

	skip = settings.value("skip", 32).toInt();
	captureInterval =	settings.value("captureInterval",	25).toInt();
	setMaxLuminance(	settings.value("maxLuminance",		1.0).toDouble());
	setMinLuminance(	settings.value("minLuminance",		0.0).toDouble());
	setChromaBoost(		settings.value("chromaBoost",		1.0).toDouble());
	setLumaBoost(		settings.value("lumaBoost",			1.2).toDouble());
	setCenterSlowness(	settings.value("centerSlowness",	5.0).toDouble());
	setSideSlowness(	settings.value("sideSlowness",		8.0).toDouble());
	
	settings.endGroup();

	emit syncParamsChanged();
	emit captureParamsChanged();
}

void Huestacean::WriteSettings()
{
	QSettings settings;
	settings.beginGroup("entertainment");

	settings.setValue("skip",				(int) skip);
	settings.setValue("captureInterval",	captureInterval);
	settings.setValue("maxLuminance",		getMaxLuminance());
	settings.setValue("minLuminance",		getMinLuminance());
	settings.setValue("chromaBoost",		getChromaBoost());
	settings.setValue("lumaBoost",			getLumaBoost());
	settings.setValue("centerSlowness",		getCenterSlowness());
	settings.setValue("sideSlowness",		getSideSlowness());

	settings.endGroup();
}

void Huestacean::ResetSettings()
{
	QSettings settings;
	settings.beginGroup("entertainment");
	settings.remove("");
	settings.endGroup();

	ReadSettings();
}

void Huestacean::pressedEnter()
{
	extern QQmlApplicationEngine* engine;
	QObject* rootObject = engine->rootObjects().first();
	QObject* qmlObject = rootObject->findChild<QObject*>("window");

	QKeyEvent *event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
	QCoreApplication::postEvent(rootObject, event);

	event = new QKeyEvent(QEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier);
	QCoreApplication::postEvent(rootObject, event);
}

int Huestacean::getMessageSendElapsed()
{
    return -1;
}

void Huestacean::setCaptureInterval(int interval)
{
    captureInterval = interval;
#if !ANDROID && !__APPLE__
    if (framegrabber)
    {
        framegrabber->setFrameChangeInterval(std::chrono::milliseconds(captureInterval));
    }
#endif
    emit captureParamsChanged();
}

void Huestacean::detectMonitors()
{
    activeMonitorIndex = 0;

#if !ANDROID

    std::vector<SL::Screen_Capture::Monitor> mons = SL::Screen_Capture::GetMonitors();
    foreach(QObject* monitor, monitors)
    {
        monitor->deleteLater();
    }
    monitors.clear();

    for (SL::Screen_Capture::Monitor& mon : mons)
    {
        Monitor* newMonitor = new Monitor(this, mon.Id, mon.Height, mon.Width, mon.OffsetX, mon.OffsetY, QString::fromUtf8(mon.Name, sizeof(mon.Name)), mon.Scaling);
        monitors.push_back(newMonitor);
    }
#endif

    emit monitorsChanged();
}

//A color sample in CIE L*C*h°
struct LChSample
{
    double L; //L*, the lightness
    double C; //C*, the chromaticity / "saturation"
    double h; //h°, the hue

	double maxRGB; //highest R,G,B value
};

void Huestacean::startScreenSync(EntertainmentGroup* eGroup)
{
    if(eGroup == nullptr) {
        qWarning() << "null eGroup!!!!!!";
        return;
    }

    syncing = true;
    emit syncingChanged();

    runSync(eGroup);
	eGroup->startStreaming([&](const EntertainmentLight& light, double oldL, double oldC, double oldh, double& L, double& C, double& h, double& minBrightness, double& maxBrightness, double& brightnessBoost)->bool {

		screenLock.lockForRead();

		if (screenSyncScreen.width == 0 || screenSyncScreen.height == 0)
		{
			screenLock.unlock();
			return false;
		}

		const double width = std::max(1.0 - std::abs(light.x), 0.3);
		const double height = std::max(1.0 - std::abs(light.z), 0.3);

		const int minX = round((std::max(light.x - width, -1.0) + 1.0) * screenSyncScreen.width / 2.0);
		const int maxX = round((std::min(light.x + width, 1.0) + 1.0) * screenSyncScreen.width / 2.0);

		const int minY = round((std::max(light.z - height, -1.0) + 1.0) * screenSyncScreen.height / 2.0);
		const int maxY = round((std::min(light.z + height, 1.0) + 1.0) * screenSyncScreen.height / 2.0);

		std::vector<LChSample> LChSamples((maxY - minY)*(maxX - minX));

		qint64 samples = 0;

		for (int y = minY; y < maxY; ++y)
		{
			int rowOffset = y * screenSyncScreen.width;
			for (int x = minX; x < maxX; ++x)
			{
				LChSample& sample = LChSamples[(y - minY) * (maxX - minX) + (x - minX)];

				double dR = (double)screenSyncScreen.screen[rowOffset + x].R / screenSyncScreen.screen[rowOffset + x].samples / 255.0;
				double dG = (double)screenSyncScreen.screen[rowOffset + x].G / screenSyncScreen.screen[rowOffset + x].samples / 255.0;
				double dB = (double)screenSyncScreen.screen[rowOffset + x].B / screenSyncScreen.screen[rowOffset + x].samples / 255.0;

				samples += screenSyncScreen.screen[rowOffset + x].samples;

				double X, Y, Z;

				sample.maxRGB = std::max(dR, std::max(dG, dB));
				Color::rgb_to_XYZ(dR, dG, dB, X, Y, Z);
				Color::XYZ_to_LCh(X, Y, Z, sample.L, sample.C, sample.h);
			}
		}

		screenLock.unlock();

		if (samples == 0)
		{
			//not ready!
			return false;
		}

		auto& chromaSamples = LChSamples;
		auto lumaSamples = LChSamples;

		constexpr double CHROMA_L_CUTOFF = 0.05;
		constexpr double CHROMA_C_CUTOFF = 0.01;

		//Filter chroma samples
		{
			//Remove the least-saturated 25% of colors
			std::sort(chromaSamples.begin(), chromaSamples.end(), [&CHROMA_L_CUTOFF](const LChSample & a, const LChSample & b) -> bool {
				return a.L < CHROMA_L_CUTOFF ? a.L > b.L : a.C > b.C;
			});

			//Iterate through samples until we find the first very unsaturated or very dim one, or we reach 75%. Trim the remaining
			int i;
			for (i = 0; i < chromaSamples.size(); ++i)
			{
				if (chromaSamples[i].L < CHROMA_L_CUTOFF || chromaSamples[i].C < CHROMA_C_CUTOFF)
				{
					break;
				}
			}

			size_t maxLeft = chromaSamples.size() * (3.0 / 4.0);
			size_t minLeft = chromaSamples.size() * (1.0 / 4.0);
			size_t finalSize = std::max(std::min(chromaSamples.size() - i, maxLeft), minLeft);

			chromaSamples.resize(finalSize);
		}

		//Filter luma samples
		{
			//Remove the least-bright 75% of colors
			std::sort(lumaSamples.begin(), lumaSamples.end(), [](const LChSample & a, const LChSample & b) -> bool {
				return a.L > b.L;
			});


			//Iterate through samples until we find the first very dim one, or we reach 75%. Trim the remaining
			int i;
			for (i = 0; i < lumaSamples.size(); ++i)
			{
				if (lumaSamples[i].L < 0.05)
				{
					break;
				}
			}

			size_t maxLeft = lumaSamples.size() * (3.0 / 4.0);
			size_t minLeft = lumaSamples.size() * (1.0 / 4.0);
			size_t finalSize = std::max(std::min(lumaSamples.size() - i, maxLeft), minLeft);

			lumaSamples.resize(finalSize);
		}

		//Determine the mean of the colors
		auto getMean = [&chromaSamples, &lumaSamples, &CHROMA_L_CUTOFF](LChSample& mean)
		{
			mean.L = 0;
			mean.C = 0;
			mean.h = 0;
			mean.maxRGB = 0;

			for (const LChSample& sample : lumaSamples)
			{
				mean.L += sample.L;
				mean.maxRGB += sample.maxRGB;
			}

			double a = 0;
			double b = 0;
			for (const LChSample& sample : chromaSamples)
			{
				//very dim samples cannot contribute color,
				//otherwise we end up with weird color casts on dim colors
				if (sample.L > CHROMA_L_CUTOFF)
				{
					mean.C += sample.C;

					a += sin(sample.h);
					b += cos(sample.h);
				}
			}

			mean.L /= (double)lumaSamples.size();
			mean.C /= (double)chromaSamples.size();
			mean.h = atan2(a, b);
			mean.maxRGB /= (double)lumaSamples.size();
		};

		LChSample mean;
		getMean(mean);

		mean.C *= getChromaBoost();

		double slowness = Utility::lerp(getCenterSlowness(), getSideSlowness(), std::abs(light.x));

		L = (oldL * (slowness - 1.0) + mean.L) / slowness;
		C = (oldC * (slowness - 1.0) + mean.C) / slowness;

		constexpr double WHITE_C_CUTOFF = 1.5;
		constexpr double WHITE_L_CUTOFF = 8.0;

		if (mean.C < CHROMA_C_CUTOFF || (mean.C < 1.5 && mean.L > WHITE_L_CUTOFF))
		{
			//if we're extremely desaturated, use the last hue
			//prevents odd changes in color when taking hard to turns to black or white
			h = oldh;
		}
		else
		{
			mean.h = std::fmod(mean.h + 2 * Color::PI, 2 * Color::PI);

			double tempOldh = oldh;

			if (std::abs(oldh - mean.h) > Color::PI) {
				if (oldh > mean.h) {
					mean.h += 2 * Color::PI;
				}
				else {
					tempOldh += 2 * Color::PI;
				}
			}

			h = (tempOldh * (slowness - 1.0) + mean.h) / slowness;
			h = std::fmod(h + 2 * Color::PI, 2 * Color::PI);
		}

		minBrightness = getMinLuminance();
		maxBrightness = getMaxLuminance();
		brightnessBoost = getLumaBoost();

        return true;
    });
}
void Huestacean::stopScreenSync()
{
    //TODO: this seems like a flaw...
    for (QObject* Obj : bridgeDiscovery->getModel())
    {
        HueBridge* bridge = qobject_cast<HueBridge*>(Obj);
        if (bridge != nullptr)
        {
            for (auto& group : bridge->EntertainmentGroups)
            {
                if (group->isStreaming)
                {
                    group->stopStreaming();
                }
            }
        }
    }
}

void Huestacean::refreshGroups()
{
    for (QObject* Obj : bridgeDiscovery->getModel())
    {
        HueBridge* bridge = qobject_cast<HueBridge*>(Obj);
        if (bridge != nullptr)
        {
            for (auto& group : bridge->EntertainmentGroups)
            {
                if (group->isStreaming)
                {
                    //refuse to modify the groups list if one is syncing right now (causes havoc at the moment)
                    return;
                }
            }
        }
    }

    for (QObject* Obj : bridgeDiscovery->getModel())
    {
        HueBridge* bridge = qobject_cast<HueBridge*>(Obj);
        if (bridge != nullptr)
        {
            bridge->requestGroups();
        }
    }
}

void Huestacean::setActiveMonitor(int index)
{
    activeMonitorIndex = index;
}

void Huestacean::updateEntertainmentGroups()
{
    entertainmentGroups.clear();

    for (QObject* Obj : bridgeDiscovery->getModel())
    {
        HueBridge* bridge = qobject_cast<HueBridge*>(Obj);
        if (bridge != nullptr)
        {
            for (auto& group : bridge->EntertainmentGroups)
            {
                entertainmentGroups.push_back(group);
            }
        }
    }

    emit entertainmentGroupsChanged();
}

void Huestacean::connectBridges()
{
    for (QObject* Obj : bridgeDiscovery->getModel())
    {
        HueBridge* bridge = qobject_cast<HueBridge*>(Obj);
        if (bridge != nullptr)
        {
            connect(bridge, SIGNAL(entertainmentGroupsChanged()),
                this, SLOT(updateEntertainmentGroups()));
            connect(bridge, SIGNAL(lightsChanged()),
                this, SIGNAL(entertainmentGroupsChanged()));
        }
    }
}

void Huestacean::runSync(EntertainmentGroup* eGroup)
{
    HueBridge* bridge = eGroup->bridgeParent();
    Q_ASSERT(bridge != nullptr);

    if (bridge->username.isEmpty() || bridge->clientkey.isEmpty())
    {
        qWarning() << "Can't start entertainment thread unless we're logged into the bridge";
        return;
    }

    if (streamingGroup != nullptr && streamingGroup != eGroup)
    {
        disconnect(streamingGroup, &EntertainmentGroup::isStreamingChanged, this, &Huestacean::isStreamingChanged);
        disconnect(streamingGroup, &EntertainmentGroup::destroyed, this, &Huestacean::streamingGroupDestroyed);

        streamingGroup->stopStreaming();
        streamingGroup = nullptr;
    }

    streamingGroup = eGroup;
    connect(streamingGroup, &EntertainmentGroup::isStreamingChanged, this, &Huestacean::isStreamingChanged);
    connect(streamingGroup, &EntertainmentGroup::destroyed, this, &Huestacean::streamingGroupDestroyed);

#if !ANDROID && !__APPLE__
    int monitorId = monitors[activeMonitorIndex]->id;

    if (framegrabber)
    {
        framegrabber.reset();
    }

	const int ResMultiplier = 1;
    const int WidthBuckets = 16 * ResMultiplier;
    const int HeightBuckets = 9 * ResMultiplier;

    framegrabber = SL::Screen_Capture::CreateCaptureConfiguration([monitorId]() {
        auto allMonitors = SL::Screen_Capture::GetMonitors();

        std::vector<SL::Screen_Capture::Monitor> chosenMonitor;
        for (auto& monitor : allMonitors)
        {
            if (monitor.Id == monitorId)
            {
                chosenMonitor.push_back(monitor);
                break;
            }
        }
        
        return chosenMonitor;
    })->onNewFrame([&, WidthBuckets, HeightBuckets](const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Monitor& monitor) {
        Q_UNUSED(monitor);

        static QElapsedTimer timer;

        const int Height = SL::Screen_Capture::Height(img);
        const int Width = SL::Screen_Capture::Width(img);

        const int RowPadding = SL::Screen_Capture::RowPadding(img);
        const int Pixelstride = img.Pixelstride;

        const int ScreenWidth = WidthBuckets;
        const int ScreenHeight = HeightBuckets;

        //const int numPixels = Height * Width;

        screenLock.lockForWrite();
        //if (ScreenWidth*ScreenHeight != eScreen.screen.size())
        //{
        //    eScreen.screen.resize(ScreenWidth * ScreenHeight);
        //}

        auto& screen = screenSyncScreen.screen;
        if (screen.size() != ScreenWidth * ScreenHeight)
        {
            screen.resize(ScreenWidth * ScreenHeight);
            screenSyncScreen.width = ScreenWidth;
            screenSyncScreen.height = ScreenHeight;
        }
        std::fill(screen.begin(), screen.end(), PixelBucket());

        //no pixel skip if mipmap generation is working on this platform
        int s;
        if (Width != SL::Screen_Capture::Width(monitor))
        {
            s = 0;
            if (!mipMapGenerationEnabled)
            {
                mipMapGenerationEnabled = true;
                emit mipMapChanged();
            }
        }
        else
        {
            s = skip;
        }

        const unsigned char* src = SL::Screen_Capture::StartSrc(img);
        for (int y = 0; y < Height; y += 1 + s)
        {
            src = SL::Screen_Capture::StartSrc(img) + (Pixelstride * Width + RowPadding)*y;
            int screenY = static_cast<int>((static_cast<uint64_t>(y) * static_cast<uint64_t>(ScreenHeight)) / static_cast<uint64_t>(Height));

            for (int x = 0; x < Width; x += 1 + s)
            {
                const int screenX = static_cast<int>((static_cast<uint64_t>(x) * static_cast<uint64_t>(ScreenWidth)) / static_cast<uint64_t>(Width));
                const int screenPos = screenY * screenSyncScreen.width + screenX;
                screen[screenPos].B += (src[0]);
                screen[screenPos].G += (src[1]);
                screen[screenPos].R += (src[2]);
                ++screen[screenPos].samples;
                src += Pixelstride * (1 + s);
            }
        }

        screenLock.unlock();

        frameReadElapsed = timer.restart();
        //emit frameReadElapsedChanged();

    })->start_capturing();

    framegrabber->pause();
    framegrabber->setFrameChangeInterval(std::chrono::milliseconds(captureInterval));
    framegrabber->setMouseChangeInterval(std::chrono::milliseconds(100000));
    framegrabber->setMipLevel(static_cast<int>(std::min(log2(monitors[activeMonitorIndex]->width / WidthBuckets), log2(monitors[activeMonitorIndex]->height / HeightBuckets))));
#elif ANDROID
    QAndroidJniObject::callStaticMethod<void>("com/huestacean/Huestacean", "StartCapture");
#elif __APPLE__
    int monitorId = monitors[activeMonitorIndex]->id;
    macScreenCapture::start(monitorId, [&](void* srcV, int rowStride, int Width, int Height){
        const int ScreenWidth = 16;
        const int ScreenHeight = 9;
        
        screenLock.lockForWrite();
        
        auto& screen = screenSyncScreen.screen;
        if (screen.size() != ScreenWidth * ScreenHeight)
        {
            screen.resize(ScreenWidth * ScreenHeight);
            screenSyncScreen.width = ScreenWidth;
            screenSyncScreen.height = ScreenHeight;
        }
        std::fill(screen.begin(), screen.end(), PixelBucket());
        
        const int Pixelstride = 4;
        const int padding = rowStride - Pixelstride * Width;
        
        /*
        for(int y = 0; y < ScreenHeight; y++)
        {
            for(int x = 0; x < ScreenWidth; x++)
            {
                unsigned const char* src = reinterpret_cast<unsigned const char*>(srcV) + rowStride*y*(Height/ScreenHeight) + Pixelstride*x*(Width/ScreenWidth);
                const int screenPos = y * screenSyncScreen.width + x;
                screen[screenPos].B += (src[0]);
                screen[screenPos].G += (src[1]);
                screen[screenPos].R += (src[2]);
                ++screen[screenPos].samples;
            }
        }*/
        
        unsigned const char* src = reinterpret_cast<unsigned const char*>(srcV);
        for (int y = 0; y < Height; y += 1)
        {
            int screenY = static_cast<int>((static_cast<uint64_t>(y) * static_cast<uint64_t>(ScreenHeight)) / static_cast<uint64_t>(Height));
            
            for (int x = 0; x < Width; x += 1)
            {
                const int screenX = static_cast<int>((static_cast<uint64_t>(x) * static_cast<uint64_t>(ScreenWidth)) / static_cast<uint64_t>(Width));
                const int screenPos = screenY * screenSyncScreen.width + screenX;
                screen[screenPos].B += (src[0]);
                screen[screenPos].G += (src[1]);
                screen[screenPos].R += (src[2]);
                ++screen[screenPos].samples;
                src += Pixelstride;
                
                //qDebug() << "R" << src[2];
                //qDebug() << "G" << src[1];
                //qDebug() << "B" << src[0];
            }
            
            src += padding;
        }
        
        static QElapsedTimer timer;
        frameReadElapsed = timer.restart();
        //emit frameReadElapsedChanged();
        
        screenLock.unlock();
        
    });
#endif
}

#if ANDROID
extern "C" {
JNIEXPORT void JNICALL
Java_com_huestacean_Huestacean_handleFrame( JNIEnv* env, jobject thiz, jint Width, jint Height)
{
    huestaceanLock.lock();
    if(huestaceanInstance == nullptr)
    {
        huestaceanLock.unlock();
        return;
    }

    huestaceanInstance->androidHandleFrame(Width, Height);

    huestaceanLock.unlock();
}

void Huestacean::androidHandleFrame(int Width, int Height)
{
    if(!syncing) return;

    const int ScreenWidth = 16;
    const int ScreenHeight = 9;

    screenLock.lockForWrite();

    auto& screen = screenSyncScreen.screen;
    if (screen.size() != ScreenWidth * ScreenHeight)
    {
        screen.resize(ScreenWidth * ScreenHeight);
        screenSyncScreen.width = ScreenWidth;
        screenSyncScreen.height = ScreenHeight;
    }
    std::fill(screen.begin(), screen.end(), PixelBucket());

    static QByteArray buf;
    buf.resize(Width*Height*3);

    const int RowPadding = 0;
    const int Pixelstride = 4;

    glReadPixels(0, 0, Width, Height, GL_RGBA, GL_UNSIGNED_BYTE, buf.data());

    const char* src = buf.data();
    for (int y = 0; y < Height; y += 1)
    {
        int screenY = static_cast<int>((static_cast<uint64_t>(y) * static_cast<uint64_t>(ScreenHeight)) / static_cast<uint64_t>(Height));

        for (int x = 0; x < Width; x += 1)
        {
            const int screenX = static_cast<int>((static_cast<uint64_t>(x) * static_cast<uint64_t>(ScreenWidth)) / static_cast<uint64_t>(Width));
            const int screenPos = screenY * screenSyncScreen.width + screenX;
            screen[screenPos].R += (src[0]);
            screen[screenPos].G += (src[1]);
            screen[screenPos].B += (src[2]);
            ++screen[screenPos].samples;
            src += Pixelstride;
        }
    }

    screenLock.unlock();

    static QElapsedTimer timer;
    frameReadElapsed = timer.restart();
    //emit frameReadElapsedChanged();
}
}
#endif

void Huestacean::isStreamingChanged(bool isStreaming)
{
#if !ANDROID && !__APPLE__
    if (isStreaming)
    {
        if (framegrabber)
        {
            framegrabber->resume();
        }
    }
    else
    {
        if (framegrabber)
        {
            framegrabber->pause();
        }
    }
#endif
#if __APPLE__
    if (!isStreaming)
        macScreenCapture::stop();
#endif

    syncing = isStreaming;
    emit syncingChanged();
}

void Huestacean::streamingGroupDestroyed()
{
    streamingGroup = nullptr;
}

ScreenSyncImageProvider::ScreenSyncImageProvider(Huestacean* parent)
    : QQuickImageProvider(QQmlImageProviderBase::Image), huestaceanParent(parent)
{

}

QImage ScreenSyncImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(id);

    huestaceanParent->screenLock.lockForRead();
    ScreenSyncScreen& screen = huestaceanParent->screenSyncScreen;

    if (screen.screen.size() == 0)
    {
        huestaceanParent->screenLock.unlock();

        if (size)
        {
            size->setWidth(1);
            size->setHeight(1);
        }

        return QImage(1, 1, QImage::Format_RGB16);
    }

    if (size)
    {
        size->setWidth(screen.width);
        size->setHeight(screen.height);
    }

    QImage img = QImage(screen.width, screen.height, QImage::Format_RGB16);
    int pixel = 0;
    for (int y = 0; y < screen.height; ++y)
    {
        for (int x = 0; x < screen.width; ++x)
        {
            if (screen.screen[pixel].samples == 0)
            {
                img.setPixelColor(QPoint(x, y), QColor(0, 0, 0));
            }
            else
            {
                /*
                qDebug() << screen.screen[pixel].samples;
                qDebug() << "R" << screen.screen[pixel].R / screen.screen[pixel].samples;
                qDebug() << "G" << screen.screen[pixel].G / screen.screen[pixel].samples;
                qDebug() << "B" << screen.screen[pixel].B / screen.screen[pixel].samples;
                 */
                
                img.setPixelColor(QPoint(x, y),
                    QColor(
                        screen.screen[pixel].R / screen.screen[pixel].samples,
                        screen.screen[pixel].G / screen.screen[pixel].samples,
                        screen.screen[pixel].B / screen.screen[pixel].samples
                    ));
            }

            ++pixel;
        }
    }

    //i.setPixelColor(QPoint())

    if(requestedSize.isValid())
        img = img.scaled(requestedSize, Qt::IgnoreAspectRatio, Qt::FastTransformation);

    //p.scaled

    huestaceanParent->screenLock.unlock();
    return img;
}



#if 0

#endif
