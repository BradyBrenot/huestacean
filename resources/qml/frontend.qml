pragma Singleton
import Huestacean.Frontend 1.0
import QtRemoteObjects 5.12
import QtQuick 2.12

FrontendQmlReplica {
    node: Node { registryUrl: "local:switch" }
}