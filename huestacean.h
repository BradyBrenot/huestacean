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

#include "ScreenCapture.h"

extern QNetworkAccessManager qnam;

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

class Huestacean : public QObject
{
    Q_OBJECT

    Q_PROPERTY(BridgeDiscovery* bridgeDiscovery READ getBridgeDiscovery NOTIFY hueInit)
    Q_PROPERTY(QList<QObject*> monitorsModel READ getMonitorsModel NOTIFY monitorsChanged)
    Q_PROPERTY(QList<QObject*> entertainmentGroupsModel READ getEntertainmentGroupsModel NOTIFY entertainmentGroupsChanged)
    Q_PROPERTY(bool syncing MEMBER syncing NOTIFY syncingChanged)

    Q_PROPERTY(int frameReadElapsed READ getFrameReadElapsed NOTIFY frameReadElapsedChanged)
    Q_PROPERTY(int messageSendElapsed READ getMessageSendElapsed NOTIFY messageSendElapsedChanged)

    Q_PROPERTY(int skip READ getSkip WRITE setSkip NOTIFY captureParamsChanged)
    Q_PROPERTY(int captureInterval READ getCaptureInterval WRITE setCaptureInterval NOTIFY captureParamsChanged)

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

signals:
    void hueInit();

private:
    BridgeDiscovery* bridgeDiscovery;

    QList<Monitor*> monitors;
    QList<EntertainmentGroup*> entertainmentGroups;

    /////////////////////////////////////////////////////////////////////////////
    // temporary home of screen sync
public:
    Q_INVOKABLE void detectMonitors();
    Q_INVOKABLE void startScreenSync(EntertainmentGroup* eGroup);
    Q_INVOKABLE void stopScreenSync();

signals:
    void monitorsChanged();
    void entertainmentGroupsChanged();
    void syncingChanged();

    void frameReadElapsedChanged();
    void messageSendElapsedChanged();

    void captureParamsChanged();

public slots:
    void setActiveMonitor(int index);
    void updateEntertainmentGroups();
    void connectBridges();

private:
    int activeMonitorIndex;
    bool syncing;

    /////////////////////////////////////////////////////////////////////
    ///ENTERTAINMENT ------------------
    void runSync(EntertainmentGroup* eGroup);
    class EntertainmentCommThread* eThread;
    QMutex eThreadMutex;
    friend class EntertainmentImageProvider;
    class EntertainmentImageProvider* eImageProvider;
    std::shared_ptr<SL::Screen_Capture::IScreenCaptureManager> framegrabber;

    QAtomicInteger<qint64> frameReadElapsed;
    QAtomicInteger<int> skip;
    int captureInterval;

private slots:
    void entertainmentThreadFinished();
    ///END ENTERTAINMENT ------------------
};

//-----------------------------------------------
///ENTERTAINMENT ------------------

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
struct EntertainmentScreen
{
    std::vector<PixelBucket> screen;
    int width;
    int height;

    EntertainmentScreen()
        : screen(), width(0), height(0)
    {

    }

    EntertainmentScreen(int inWidth, int inHeight)
        : screen(), width(inWidth), height(inHeight)
    {
        screen.resize(inWidth * inHeight);
    }

    EntertainmentScreen& operator=(const EntertainmentScreen& other)
    {
        screen = other.screen;
        width = other.width;
        height = other.height;
        return *this;
    }
};

class EntertainmentCommThread : public QThread
{
    Q_OBJECT

public:
    explicit EntertainmentCommThread(QObject *parent, QString inUsername, QString inClientkey, QString address, EntertainmentGroup* inGroup);

    void run() override;
    void threadsafe_setScreen(const EntertainmentScreen& inScreen);
    EntertainmentScreen& getScreenForPixmap();
    void stop();

    QReadWriteLock LightColorsLock;
    QHash<QString, QColor> LightColors;

    QAtomicInteger<qint64> messageSendElapsed;

signals:
    void messageSendElapsedChanged();
    void lightColorsChanged();

private:
    QMutex screenMutex;
    EntertainmentScreen screen;
    QString username;
    QString clientkey;
    std::atomic<bool> stopRequested;
    QString address;

    Huestacean* huestacean;
    EntertainmentGroup eGroup;
};

class EntertainmentImageProvider : public QQuickImageProvider
{
public:
    EntertainmentImageProvider(Huestacean* parent);
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);

private:
    Huestacean * huestaceanParent;
};
///END ENTERTAINMENT ------------------
//-----------------------------------------------