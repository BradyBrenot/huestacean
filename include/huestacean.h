#pragma once

#include <QObject>
#include <QStringListModel>
#include <QQuickImageProvider>
#include <QReadWriteLock>
#include <QAtomicInteger>
#include <QColor>

#include "huebridge.h"
#include "objectmodel.h"
#include "bridgediscovery.h"

#if !ANDROID
#include "screen_capture_lite/include/ScreenCapture.h"
#else
#include <jni.h>
#endif


extern QNetworkAccessManager* qnam;

//-----------------------------------------------
//Screen sync -----------------------------------

class Monitor : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int id MEMBER id NOTIFY propertiesChanged)
    Q_PROPERTY(int height MEMBER height NOTIFY propertiesChanged)
    Q_PROPERTY(int width MEMBER width NOTIFY propertiesChanged)
    Q_PROPERTY(int offsetX MEMBER offsetX NOTIFY propertiesChanged)
    Q_PROPERTY(int offsetY MEMBER offsetY NOTIFY propertiesChanged)
    Q_PROPERTY(QString name MEMBER name NOTIFY propertiesChanged)
    Q_PROPERTY(float scaling MEMBER scaling NOTIFY propertiesChanged)
    Q_PROPERTY(QString asString READ toString NOTIFY propertiesChanged)

public:
    explicit Monitor(QObject *parent = nullptr) 
        : QObject(parent)
        , id(INT32_MAX), height(0), width(0), offsetX(0), offsetY(0), name(), scaling(1.0f)
    {
        emit propertiesChanged();
    }

    explicit Monitor(QObject *parent, int inId, int inHeight, int inWidth, int inOffsetX, int inOffsetY, QString inName, float inScaling)
        : QObject(parent)
        , id(inId), height(inHeight), width(inWidth), offsetX(inOffsetX), offsetY(inOffsetY), name(inName), scaling(inScaling)
    {
        emit propertiesChanged();
    }

    QString toString() {
        return QString("%2 (%3x%4)").arg(QString::number(id), QString::number(width), QString::number(height));
    }

    int id;
    int height;
    int width;
    int offsetX;
    int offsetY;
    QString name;
    float scaling;

signals:
    void propertiesChanged();
};

struct PixelBucket
{
    uint64_t R;
    uint64_t G;
    uint64_t B;
    uint64_t samples;

    PixelBucket()
        : R(0), G(0), B(0), samples(0)
    {

    }
};

//Screen broken down into buckets
struct ScreenSyncScreen
{
    std::vector<PixelBucket> screen;
    int width;
    int height;

    ScreenSyncScreen()
        : screen(), width(0), height(0)
    {

    }

    ScreenSyncScreen(int inWidth, int inHeight)
        : screen(), width(inWidth), height(inHeight)
    {
        screen.resize(inWidth * inHeight);
    }

    ScreenSyncScreen& operator=(const ScreenSyncScreen& other)
    {
        screen = other.screen;
        width = other.width;
        height = other.height;
        return *this;
    }
};

class ScreenSyncImageProvider : public QQuickImageProvider
{
public:
    ScreenSyncImageProvider(class Huestacean* parent);
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);

private:
    Huestacean * huestaceanParent;
};
// END SCREEN SYNC ------------------------------
//-----------------------------------------------

class Huestacean : public QObject
{
    Q_OBJECT

    Q_PROPERTY(BridgeDiscovery* bridgeDiscovery READ getBridgeDiscovery NOTIFY hueInit)
    Q_PROPERTY(QList<QObject*> monitorsModel READ getMonitorsModel NOTIFY monitorsChanged)
    Q_PROPERTY(QList<QObject*> entertainmentGroupsModel READ getEntertainmentGroupsModel NOTIFY entertainmentGroupsChanged)
    Q_PROPERTY(bool syncing MEMBER syncing NOTIFY syncingChanged)
    Q_PROPERTY(bool mipMapGenerationEnabled MEMBER mipMapGenerationEnabled NOTIFY mipMapChanged)

    Q_PROPERTY(int frameReadElapsed READ getFrameReadElapsed NOTIFY frameReadElapsedChanged)
    Q_PROPERTY(int messageSendElapsed READ getMessageSendElapsed NOTIFY messageSendElapsedChanged)

    Q_PROPERTY(int skip READ getSkip WRITE setSkip NOTIFY captureParamsChanged)
    Q_PROPERTY(int captureInterval READ getCaptureInterval WRITE setCaptureInterval NOTIFY captureParamsChanged)

    Q_PROPERTY(double minLuminance READ getMinLuminance WRITE setMinLuminance NOTIFY syncParamsChanged)
    Q_PROPERTY(double maxLuminance READ getMaxLuminance WRITE setMaxLuminance NOTIFY syncParamsChanged)
    Q_PROPERTY(double chromaBoost READ getChromaBoost WRITE setChromaBoost NOTIFY syncParamsChanged)

    Q_PROPERTY(double centerSlowness READ getCenterSlowness WRITE setCenterSlowness NOTIFY syncParamsChanged)
    Q_PROPERTY(double sideSlowness READ getSideSlowness WRITE setSideSlowness NOTIFY syncParamsChanged)

public:
    explicit Huestacean(QObject *parent = nullptr);
    virtual ~Huestacean();

    BridgeDiscovery* getBridgeDiscovery() {
        return bridgeDiscovery;
    }
    QList<QObject*> getMonitorsModel() {
        return *(reinterpret_cast<QList<QObject*>*>(&monitors));
    }
    QList<QObject*> getEntertainmentGroupsModel() {
        return *(reinterpret_cast<QList<QObject*>*>(&entertainmentGroups));
    }

    int getFrameReadElapsed() {
        return frameReadElapsed;
    }
    int getMessageSendElapsed();

    int getSkip() {
        return skip;
    }

    void setSkip(int inSkip) {
        skip = inSkip;
        emit captureParamsChanged();
    }

    int getCaptureInterval() {
        return captureInterval;
    }

    void setCaptureInterval(int interval);

    float getMinLuminance() {
        return minLuminance / 1000.0;
    }

    void setMinLuminance(double in) {
        minLuminance = in * 1000.0;
        emit syncParamsChanged();
    }

    float getMaxLuminance() {
        return maxLuminance / 1000.0;
    }

    void setMaxLuminance(double in) {
        maxLuminance = in * 1000.0;
        emit syncParamsChanged();
    }

    float getChromaBoost() {
        return chromaBoost / 1000.0;
    }

    void setChromaBoost(double in) {
        chromaBoost = in * 1000.0;
        emit syncParamsChanged();
    }

    double getCenterSlowness() {
        return centerSlowness / 1000.0;
    }

    void setCenterSlowness(double in) {
        centerSlowness = in * 1000.0;
        emit syncParamsChanged();
    }

    double getSideSlowness() {
        return sideSlowness / 1000.0;
    }

    void setSideSlowness(double in) {
        sideSlowness = in * 1000.0;
        emit syncParamsChanged();
    }

public slots:
    void connectBridges();

signals:
    void hueInit();

private:
    BridgeDiscovery* bridgeDiscovery;

    QList<Monitor*> monitors;
    QList<EntertainmentGroup*> entertainmentGroups;

    //-----------------------------------------------
    // SCREEN SYNC ----------------------------------
public:
    Q_INVOKABLE void detectMonitors();
    Q_INVOKABLE void startScreenSync(EntertainmentGroup* eGroup);
    Q_INVOKABLE void stopScreenSync();
    Q_INVOKABLE void refreshGroups();

#if ANDROID
    void androidHandleFrame(int Width, int Height);
#endif

signals:
    void monitorsChanged();
    void entertainmentGroupsChanged();
    void syncingChanged();

    void frameReadElapsedChanged();
    void messageSendElapsedChanged();

    void captureParamsChanged();
    void syncParamsChanged();
    void mipMapChanged();

public slots:
    void setActiveMonitor(int index);
    void updateEntertainmentGroups();

private:
    int activeMonitorIndex;
    bool syncing;

    void runSync(EntertainmentGroup* eGroup);
    QMutex eThreadMutex;
    friend class ScreenSyncImageProvider;
    class ScreenSyncImageProvider* eImageProvider;

#if !ANDROID
    std::shared_ptr<SL::Screen_Capture::IScreenCaptureManager> framegrabber;
#endif

    QAtomicInteger<qint64> frameReadElapsed;
    QAtomicInteger<int> skip;
    int captureInterval;
    class EntertainmentGroup* streamingGroup;

    QReadWriteLock screenLock;
    ScreenSyncScreen screenSyncScreen;

    QAtomicInteger<qint64> minLuminance;
    QAtomicInteger<qint64> maxLuminance;
    QAtomicInteger<qint64> chromaBoost;
    QAtomicInteger<qint64> centerSlowness;
    QAtomicInteger<qint64> sideSlowness;
    QAtomicInteger<bool> mipMapGenerationEnabled;

private slots:
    void isStreamingChanged(bool isStreaming);
    void streamingGroupDestroyed();
    // END SCREEN SYNC ------------------------------
    //-----------------------------------------------
};
