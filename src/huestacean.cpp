#include "huestacean.h"
#include "entertainment.h"
#include "utility.h"

#include <QQmlApplicationEngine>
#include <QElapsedTimer>
#include <QTimer>

#include <algorithm>
#include <cmath>

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

    //ENTERTAINMENT
    skip = 32;
    captureInterval = 25;
    streamingGroup = nullptr;
    setMaxLuminance(1.0);
    setMinLuminance(0.0);
    setChromaBoost(1.0);
    mipMapGenerationEnabled = false;

    setCenterSlowness(10.0);
    setSideSlowness(20.0);

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

struct xySample
{
    double x;
    double y;
    double l; //aka "Y", the perceived brightness
    double err;
};

void Huestacean::startScreenSync(EntertainmentGroup* eGroup)
{
    if(eGroup == nullptr) {
        qWarning() << "null eGroup!!!!!!";
        return;
    }

    static const double D65_x = 0.3128;
    static const double D65_y = 0.3290;

    syncing = true;
    emit syncingChanged();

    runSync(eGroup);
    eGroup->startStreaming([&](const EntertainmentLight& light, double oldX, double oldY, double oldL, double& X, double& Y, double& L)->bool {

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

        std::vector<xySample> xySamples((maxY-minY)*(maxX-minX));

        qint64 samples = 0;

        for (int y = minY; y < maxY; ++y)
        {
            int rowOffset = y * screenSyncScreen.width;
            for (int x = minX; x < maxX; ++x)
            {
                xySample& sample = xySamples[(y - minY) * (maxX - minX) + (x - minX)];

                double dR = (double)screenSyncScreen.screen[rowOffset + x].R / screenSyncScreen.screen[rowOffset + x].samples / 255.0;
                double dG = (double)screenSyncScreen.screen[rowOffset + x].G / screenSyncScreen.screen[rowOffset + x].samples / 255.0;
                double dB = (double)screenSyncScreen.screen[rowOffset + x].B / screenSyncScreen.screen[rowOffset + x].samples / 255.0;

                samples += screenSyncScreen.screen[rowOffset + x].samples;

                Utility::rgb_to_xy(dR, dG, dB, X, Y, L);

                if (dR == 0.0 && dG == 0.0 && dB == 0.0) {
                    X = D65_x;
                    Y = D65_y;
                }

                sample.x = X;
                sample.y = Y;
                sample.l = L;
            }
        }

        screenLock.unlock();

        if (samples == 0)
        {
            //not ready!
            return false;
        }

        //Remove the least-saturated 25% of colors
        for (xySample& sample : xySamples)
        {
            sample.err = std::pow(D65_x - sample.x, 2.0) + std::pow(D65_y - sample.y, 2.0);
        }
        std::sort(xySamples.begin(), xySamples.end(), [](const xySample & a, const xySample & b) -> bool {
            return a.err > b.err;
        });
        xySamples.resize(xySamples.size() * (3.0 / 4.0));

        //Determine the mean of the colors
        auto getMean = [&xySamples](xySample& mean)
        {
            mean.x = 0;
            mean.y = 0;
            mean.l = 0;

            for (const xySample& sample : xySamples)
            {
                mean.x += sample.x;
                mean.y += sample.y;
                mean.l += sample.l;
            }

            mean.x /= (double)xySamples.size();
            mean.y /= (double)xySamples.size();
            mean.l /= (double)xySamples.size();
        };

        xySample mean;

#if 0
        getMean(mean);
        //Cut out the most-outlying 25% of colors, then figure out the mean again
        for (xySample& sample : xySamples)
        {
            sample.err = std::pow(mean.x - sample.x, 2.0) + std::pow(mean.y - sample.y, 2.0) + std::pow(mean.l - sample.l, 2.0);
        }
        std::sort(xySamples.begin(), xySamples.end(), [](const xySample & a, const xySample & b) -> bool {
            return a.err < b.err;
        });
        xySamples.resize(xySamples.size() * (3.0 / 4.0));
#endif

        getMean(mean);
        X = mean.x;
        Y = mean.y;
        L = mean.l;

        //Boost 'saturation' by boosting distance from D65... in a stupid way but what'll you do?
        double chromaBoost = getChromaBoost();
        if (chromaBoost > 1.00001)
        {
            double dist = std::sqrt(std::pow(X - D65_x, 2.0) + std::pow(Y - D65_y, 2.0));

            //if brightness is too low and we're very desaturated, use last chroma
            if (L < 0.1 && dist < 0.1)
            {
                X = oldX;
                Y = oldY;
                dist = std::sqrt(std::pow(X - D65_x, 2.0) + std::pow(Y - D65_y, 2.0));
            }

            double boostDist = std::pow(dist, 1.0 / chromaBoost);
            double diffX = (X - D65_x);
            double diffY = (Y - D65_y);
            double unitX = diffX == 0.0 ? 0.0 : diffX / std::sqrt(std::pow(diffX, 2.0) + std::pow(diffY, 2.0));
            double unitY = diffY == 0.0 ? 0.0 : diffY / std::sqrt(std::pow(diffX, 2.0) + std::pow(diffY, 2.0));

            double boostX = D65_x + unitX * boostDist;
            double boostY = D65_y + unitY * boostDist;

            double bestDist = 0;
            double testDist;

            if (boostX < 0 || boostX > 1.0 || boostY < 0 || boostY > 1.0)
            {
                if (unitX > 0.0)
                {
                    testDist = (1.0 - D65_x) / unitX;
                    if (D65_y + testDist * unitY <= 1.0 && D65_y + testDist * unitY >= 0.0)
                    {
                        bestDist = std::max(bestDist, testDist);
                    }
                }
                else
                {
                    testDist = (-D65_x) / unitX;
                    if (D65_y + testDist * unitY <= 1.0 && D65_y + testDist * unitY >= 0.0)
                    {
                        bestDist = std::max(bestDist, testDist);
                    }
                }

                if (unitY > 0.0)
                {
                    testDist = (1.0 - D65_y) / unitY;
                    if (D65_x + testDist * unitX <= 1.0 && D65_x + testDist * unitX >= 0.0)
                    {
                        bestDist = std::max(bestDist, testDist);
                    }
                }
                else
                {
                    testDist = (-D65_y) / unitY;
                    if (D65_x + testDist * unitX <= 1.0 && D65_x + testDist * unitX >= 0.0)
                    {
                        bestDist = std::max(bestDist, testDist);
                    }
                }

                boostDist = bestDist;
                boostX = D65_x + unitX * boostDist;
                boostY = D65_y + unitY * boostDist;
            }

            X = boostX;
            Y = boostY;
        }

        L = L * (getMaxLuminance() - getMinLuminance()) + getMinLuminance();

        double slowness = Utility::lerp(getCenterSlowness(), getSideSlowness(), std::abs(light.x));

        X = (oldX * (slowness - 1.0) + X) / slowness;
        Y = (oldY * (slowness - 1.0) + Y) / slowness;
        L = (oldL * (slowness - 1.0) + L) / slowness;

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

    const int WidthBuckets = 16;
    const int HeightBuckets = 9;

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
    })->onNewFrame([&](const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Monitor& monitor) {
        Q_UNUSED(monitor);

        static QElapsedTimer timer;

        const int Height = SL::Screen_Capture::Height(img);
        const int Width = SL::Screen_Capture::Width(img);

        const int RowPadding = SL::Screen_Capture::RowPadding(img);
        const int Pixelstride = img.Pixelstride;

        const int ScreenWidth = 16;
        const int ScreenHeight = 9;

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
