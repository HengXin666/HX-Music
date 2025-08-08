pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import QtQuick.Dialogs
import Qt5Compat.GraphicalEffects
import HX.Music

Item {
    id: root
    focus: true

    MusicListModel {
        id: musicListModel
    }

    DropArea {
        anchors.fill: parent
        onDropped: drop => {
            for (const urlStr of drop.urls) {
                let path = decodeURIComponent(urlStr).replace("file://", "");
                console.log("Dropped file path:", path);
                musicListModel.addFromPath(path);
            }
        }
    }

    Column {
        anchors.fill: parent

        // 表头
        Rectangle {
            height: 40
            color: "#f5f5f5"
            Row {
                spacing: 10
                anchors.verticalCenter: parent.verticalCenter
                Text {
                    width: 40
                    text: "#"
                }
                Text {
                    width: 50
                    text: "封面"
                }
                Text {
                    width: 200
                    text: "歌名"
                }
                Text {
                    width: 120
                    text: "歌手"
                }
                Text {
                    width: 150
                    text: "专辑"
                }
                Text {
                    width: 60
                    text: "时长"
                }
            }
        }

        ListView {
            id: listView
            model: musicListModel
            anchors.fill: parent
            clip: true
            delegate: Item {
                id: delegateRoot
                required property int index
                required property var model
                width: listView.width
                height: 60

                MouseArea {
                    anchors.fill: parent
                    onDoubleClicked: {
                        console.log("播放:", delegateRoot.index);
                        // MusicPlayer.play(delegateRoot.index) // 调用播放
                    }
                }

                Row {
                    spacing: 10
                    anchors.verticalCenter: parent.verticalCenter

                    // 序号
                    Text {
                        width: 40
                        text: (delegateRoot.index + 1).toString()
                        font.pixelSize: 16
                        horizontalAlignment: Text.AlignHCenter
                    }

                    // 封面
                    Item {
                        width: 50
                        height: 50
                        Rectangle {
                            anchors.fill: parent
                            color: "#0a7d92"
                            Image {
                                anchors.fill: parent
                                source: `image://imgPool/${delegateRoot.model.url}`
                                fillMode: Image.PreserveAspectCrop
                            }
                        }
                    }

                    // 歌名
                    Text {
                        text: delegateRoot.model.title
                        width: 200
                        font.pixelSize: 16
                        elide: Text.ElideRight
                    }

                    // 歌手
                    Text {
                        text: delegateRoot.model.artist
                        width: 120
                        font.pixelSize: 14
                        color: "#e62727"
                        elide: Text.ElideRight
                    }

                    // 专辑
                    Text {
                        text: delegateRoot.model.album
                        width: 150
                        font.pixelSize: 14
                        color: "#079f25"
                        elide: Text.ElideRight
                    }

                    // 时长
                    Text {
                        text: root.formatDuration(delegateRoot.model.duration)
                        width: 60
                        font.pixelSize: 14
                        horizontalAlignment: Text.AlignRight
                    }
                }

                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: 1
                    color: "#0c2554"
                }
            }
        }
    }

    // 时间格式化
    function formatDuration(seconds) {
        let min = Math.floor(seconds / 60);
        let sec = seconds % 60;
        return (min < 10 ? "0" : "") + min + ":" + (sec < 10 ? "0" : "") + sec;
    }
}
