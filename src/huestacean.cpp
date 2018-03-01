#include "huestacean.h"
#include "entertainment.h"
#include "utility.h"

#include <QQmlApplicationEngine>
#include <QElapsedTimer>

#include <algorithm>
#include <cmath>

QNetworkAccessManager qnam(nullptr);

Huestacean::Huestacean(QObject *parent) 
    : QObject(parent),
    syncing(false)
{
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

    qmlRegisterType<EntertainmentGroup>();
}

Huestacean::~Huestacean()
{
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

    
}

int Huestacean::getMessageSendElapsed()
{
    return -1;
}

void Huestacean::setCaptureInterval(int interval)
{
    captureInterval = interval;
    if (framegrabber)
    {
        framegrabber->setFrameChangeInterval(std::chrono::milliseconds(captureInterval));
    }
    emit captureParamsChanged();
}

void Huestacean::detectMonitors()
{
    activeMonitorIndex = 0;

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

    emit monitorsChanged();
}

void Huestacean::startScreenSync(EntertainmentGroup* eGroup)
{
    syncing = true;
    emit syncingChanged();

    runSync(eGroup);
    eGroup->startStreaming([&](const EntertainmentLight& light, double& X, double& Y, double& L)->bool {

        screenLock.lockForRead();

        const double width = std::max(1.0 - std::abs(light.x), 0.3);
        const double height = std::max(1.0 - std::abs(light.z), 0.3);

        const int minX = std::round((std::max(light.x - width, -1.0) + 1.0) * screenSyncScreen.width / 2.0);
        const int maxX = std::round((std::min(light.x + width, 1.0) + 1.0) * screenSyncScreen.width / 2.0);

        const int minY = std::round((std::max(light.z - height, -1.0) + 1.0) * screenSyncScreen.height / 2.0);
        const int maxY = std::round((std::min(light.z + height, 1.0) + 1.0) * screenSyncScreen.height / 2.0);

        quint64 R = 0;
        quint64 G = 0;
        quint64 B = 0;
        quint64 samples = 0;

        for (int y = minY; y < maxY; ++y)
        {
            int rowOffset = y * screenSyncScreen.width;
            for (int x = minX; x < maxX; ++x)
            {
                R += screenSyncScreen.screen[rowOffset + x].R;
                G += screenSyncScreen.screen[rowOffset + x].G;
                B += screenSyncScreen.screen[rowOffset + x].B;
                samples += screenSyncScreen.screen[rowOffset + x].samples;
            }
        }

        screenLock.unlock();

        if (samples == 0)
        {
            //not ready!
            return 0;
        }

        double dR = (double)R / samples / 255.0;
        double dG = (double)G / samples / 255.0;
        double dB = (double)B / samples / 255.0;

        Utility::rgb_to_xy(dR, dG, dB, X, Y, L);

        //Boost 'saturation' by boosting distance from D65... in a stupid way but what'll you do?
        constexpr double D65_x = 0.3128;
        constexpr double D65_y = 0.3290;

        double chromaBoost = getChromaBoost();

        double dist = std::sqrt(std::pow(X - D65_x, 2.0) + std::pow(Y - D65_y, 2.0));

        double boostDist = std::pow(dist, 1.0 / chromaBoost);
        double diffX = (X - D65_x);
        double diffY = (Y - D65_y);
        double unitX = diffX / std::sqrt(std::pow(diffX, 2.0) + std::pow(diffY, 2.0));
        double unitY = diffY / std::sqrt(std::pow(diffX, 2.0) + std::pow(diffY, 2.0));

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

        L = L * (getMaxLuminance() - getMinLuminance()) + getMinLuminance();

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

        const int numPixels = Height * Width;

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
        const int s = Width != SL::Screen_Capture::Width(monitor) ? 0 : skip;

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
        emit frameReadElapsedChanged(); //TODO: make sure this is thread-safe. Pretty sure it is from the docs I'm reading...

    })->start_capturing();

    framegrabber->pause();
    framegrabber->setFrameChangeInterval(std::chrono::milliseconds(captureInterval));
    framegrabber->setMouseChangeInterval(std::chrono::milliseconds(100000));
    framegrabber->setMipLevel(static_cast<int>(std::max(std::log2(monitors[activeMonitorIndex]->width / WidthBuckets), std::log2(monitors[activeMonitorIndex]->height / HeightBuckets))));
}

void Huestacean::isStreamingChanged(bool isStreaming)
{
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
                img.setPixelColor(QPoint(x, y), QColor());
            }
            else
            {
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