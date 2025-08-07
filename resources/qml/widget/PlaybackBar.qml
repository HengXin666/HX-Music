import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Window
import HX.Music
import "./internal"

Item {
    id: root

    property int itemHeight: 100
    height: itemHeight

    MusicController {
        id: musicController
    }

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
        ColumnLayout {
            id: centerLayout
            anchors.horizontalCenter: container.horizontalCenter
            anchors.verticalCenter: container.verticalCenter

            // 按钮
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 10
                MusicActionButton {
                    url: "qrc:/icons/previous.svg"
                    onClicked: musicController.prev()
                }
                MusicActionButton {
                    url: musicController.isPlaying ? "qrc:/icons/pause.svg"
                                                   : "qrc:/icons/play.svg"
                    onClicked: musicController.togglePause();
                }
                MusicActionButton {
                    url: "qrc:/icons/next.svg"
                    onClicked: musicController.next()
                }
            }

            // 播放条
            RowLayout {
                Text {
                    text: "00:00"
                }
                MusicProgressBar {

                }
                Text {
                    text: "66:66"
                }
            }
        }

        // 靠右
        RowLayout {
            id: rightLayout
            anchors.right: container.right
            anchors.rightMargin: 10
            anchors.verticalCenter: container.verticalCenter

            Button {
                text: "歌词"
                onClicked: lyricsState.switchWindow();
            }
        }
    }
}
