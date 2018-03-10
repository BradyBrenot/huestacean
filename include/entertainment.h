#pragma once

#include <QThread>
#include <functional>
#include "huebridge.h"

class EntertainmentLight : public Light
{
    Q_OBJECT;

    Q_PROPERTY(double x MEMBER x NOTIFY propertiesChanged)
    Q_PROPERTY(double y MEMBER y NOTIFY propertiesChanged)
    Q_PROPERTY(double z MEMBER z NOTIFY propertiesChanged)

public:
    double x;
    double y;
    double z;

    explicit EntertainmentLight() : Light(nullptr)
    {
    }

    explicit EntertainmentLight(HueBridge *parent, QString inId, double inX, double inY, double inZ) :
        Light(parent), x(inX), y(inY), z(inZ)
    {
        id = inId;
        emit propertiesChanged();
    }

    explicit EntertainmentLight(const EntertainmentLight& other) : EntertainmentLight(other.bridgeParent(), other.id, other.x, other.y, other.z)
    {
    }
};

inline bool operator==(const EntertainmentLight& a, const EntertainmentLight& b)
{
    return a.id == b.id;
}

/*
 * This WILL be called in a different thread than it was created in.
 * 
 * @param[in] EntertainmentLight    The light that we want to get the color for
 * @param[out] x                    x-value of color to apply to the light
 * @param[out] y                    y-value of color to apply to the light
 * @param[out] l                    brightness to apply to the light
 * @return                          Whether we're ready to send a color to this light. If false, an update for this light is not sent to the Bridge.
 */
typedef std::function<bool(const EntertainmentLight&, double&, double&, double&)> GetColorFunction;

class EntertainmentGroup : public BridgeObject
{
    Q_OBJECT;

    Q_PROPERTY(QString name MEMBER name NOTIFY propertiesChanged)
    Q_PROPERTY(bool isStreaming MEMBER isStreaming NOTIFY isStreamingChanged)
    Q_PROPERTY(QList<EntertainmentLight> lights MEMBER lights NOTIFY propertiesChanged)
    Q_PROPERTY(QString asString READ toString NOTIFY propertiesChanged)

public:
    explicit EntertainmentGroup(HueBridge *parent);

    explicit EntertainmentGroup()
        : EntertainmentGroup(nullptr)
    {
    }

    explicit EntertainmentGroup(const EntertainmentGroup& other)
        : EntertainmentGroup(other.bridgeParent())
    {
        name = other.name;
        lights = other.lights;
        id = other.id;
        emit propertiesChanged();
    }

    virtual ~EntertainmentGroup();

    EntertainmentGroup& operator=(const EntertainmentGroup& other)
    {
        setParent(other.parent());
        name = other.name;
        id = other.id;
        lights = other.lights;
        emit propertiesChanged();
        return *this;
    }

    QString toString() {
        return QString("%1 (%2)").arg(name, bridgeParent()->friendlyName);;
    }

    Q_INVOKABLE int numLights() { return lights.length(); }
    Q_INVOKABLE QObject* getLight(int index) { return &lights[index]; }
    Q_INVOKABLE void updateLightXZ(int index, float x, float z, bool save);

    /*
     * Ask the bridge to start streaming.
     * If the bridge web API refuses our request, emit failedToStartStreaming()
     * If the bridge web API accepted our request, but the secure UDP connection fails, emit udpConnectFailed()
     * Otherwise, isStreaming will be true and isStreamingChanged will be emitted.
     */
    void startStreaming(GetColorFunction getColorFunc);
    void stopStreaming();
    void shutDownImmediately();
    bool hasRunningThread();

    QString name;
    QList<EntertainmentLight> lights;
    bool isStreaming;

signals:
    void isStreamingChanged(bool newIsStreaming);
    void failedToStartStreaming();
    void udpConnectFailed();

private:
    void askBridgeToToggleStreaming(bool enabled);

    class EntertainmentCommThread* eThread;
    GetColorFunction getColor;

private slots:
    void entertainmentThreadConnected();
    void entertainmentThreadFinished();
    void replied(QNetworkReply *reply);
};

class EntertainmentCommThread : public QThread
{
    Q_OBJECT

public:
    explicit EntertainmentCommThread(QObject *parent, QString inUsername, QString inClientkey, QString inAddress, const EntertainmentGroup& inEGroup, GetColorFunction getColorFunc);

    void run() override;
    void stop();

    QAtomicInteger<qint64> messageSendElapsed;

signals:
    void connectFailed();
    void connected();
    void messageSendElapsedChanged();

private:
    QString username;
    QString clientkey;
    std::atomic<bool> stopRequested;
    QString address;

    QMutex eGroupMutex;
    EntertainmentGroup eGroup;
    GetColorFunction getColor;

    friend struct EntertainmentCommThreadEGroupScopedLock;
};

struct EntertainmentCommThreadEGroupScopedLock
{
    EntertainmentCommThreadEGroupScopedLock(EntertainmentCommThread* inThread)
    {
        eThread = inThread;
        eThread->eGroupMutex.lock();
    }

    ~EntertainmentCommThreadEGroupScopedLock()
    {
        eThread->eGroupMutex.unlock();
    }

    EntertainmentGroup* operator-> ()
    {
        return &eThread->eGroup;
    }

private:
    EntertainmentCommThread * eThread;
};