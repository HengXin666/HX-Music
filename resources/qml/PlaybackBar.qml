import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Controls.Material

Item {
    id: root

    property int itemHeight: 100
    height: itemHeight

    Rectangle {
        anchors.fill: parent
        color: "#7b990099"
    }

    Item {
        id: container
        anchors.fill: parent

        Text {
            id: leftText
            text: "靠左的"
            anchors.left: container.left
            anchors.leftMargin: 10
            anchors.verticalCenter: container.verticalCenter
        }

        Text {
            id: centerText
            text: "居中的"
            anchors.horizontalCenter: container.horizontalCenter
            anchors.verticalCenter: container.verticalCenter
        }

        Text {
            id: rightText
            text: "靠右的"
            anchors.right: container.right
            anchors.rightMargin: 10
            anchors.verticalCenter: container.verticalCenter
        }
    }
}
