import QtQuick
import QtAcrylic

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    AcrylicItem {
        anchors.fill: parent
    }
}
