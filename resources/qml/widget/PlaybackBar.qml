import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Window

Item {
    id: root

    property int itemHeight: 100
    height: itemHeight

    Rectangle {
        id: container
        anchors.fill: parent
        color: "#7b990099"

        // 靠左
        Text {
            id: leftLayout
            anchors.left: container.left
            anchors.leftMargin: 10
            anchors.verticalCenter: container.verticalCenter
        }

        // 居中
        RowLayout {
            id: centerLayout
            anchors.horizontalCenter: container.horizontalCenter
            anchors.verticalCenter: container.verticalCenter

            MusicProgressBar {
                
            }
        }

        // 靠右
        RowLayout {
            id: rightLayout
            anchors.right: container.right
            anchors.rightMargin: 10
            anchors.verticalCenter: container.verticalCenter

            LyricsButton {
                text: "歌词"
            }
        }
    }
}
