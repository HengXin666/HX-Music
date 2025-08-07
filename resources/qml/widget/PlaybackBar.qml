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
        color: "#3f000000"

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
                    url: musicController.isPlaying ? "qrc:/icons/pause.svg" : "qrc:/icons/play.svg"
                    onClicked: musicController.togglePause()
                }
                MusicActionButton {
                    url: "qrc:/icons/next.svg"
                    onClicked: musicController.next()
                }
            }

            // 播放条
            RowLayout {
                id: musicProgressBarLayout
                function formatTime(seconds: int): string {
                    seconds = Math.max(0, Math.floor(seconds)); // 保证非负整数

                    const hrs = Math.floor(seconds / 3600);
                    const mins = Math.floor((seconds % 3600) / 60);
                    const secs = seconds % 60;

                    function pad(num) {
                        return num < 10 ? "0" + num : "" + num;
                    }

                    if (hrs > 0) {
                        return pad(hrs) + ":" + pad(mins) + ":" + pad(secs);
                    } else {
                        return pad(mins) + ":" + pad(secs);
                    }
                }

                Text {
                    id: musicNowPosText
                    text: "--:--"
                    color: "#c2c2c2"
                    Connections {
                        target: SignalBusSingleton
                        // 绑定信号: 播放位置变化
                        function onMusicPlayPosChanged(pos: int) {
                            musicNowPosText.text = musicProgressBarLayout.formatTime(pos / 1000);
                        }
                    }
                }
                MusicProgressBar {}
                Text {
                    id: musicLengthText
                    text: "--:--"
                    color: "#c2c2c2"
                    Connections {
                        target: SignalBusSingleton
                        // 绑定信号: 更新歌曲
                        function onNewSongLoaded(_) {
                            musicLengthText.text = musicProgressBarLayout.formatTime(
                                musicController.getLengthInMilliseconds() / 1000);
                        }
                    }
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
                onClicked: lyricsState.switchWindow()
            }
        }
    }
}
