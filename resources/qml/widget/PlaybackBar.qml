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

        RowLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 20

            // 靠左
            RowLayout {
                id: leftLayout
                Layout.minimumWidth: 200
                Layout.alignment: Qt.AlignVCenter

                // 封面
                Item {
                    Layout.preferredWidth: 60
                    Layout.preferredHeight: 60
                    MusicActionButton {
                        anchors.fill: parent
                        url: "qrc:/icons/audio.svg"
                    }
                }

                ColumnLayout {
                    Layout.alignment: Qt.AlignVCenter

                    // 滚动文本: 歌曲名 - 歌手... (可点击)
                    LoopingScrollingText {
                        Layout.fillWidth: true
                        itemData: [
                            { text: "__xxx__", onClick: function() { console.log("点击xxx") } },
                            { text: "-", onClick: function() { console.log("点击2") } },
                            { text: "__yyy__", onClick: function() { console.log("点击yyy") } },
                            { text: "、", onClick: function() { console.log("点击4") } },
                            { text: "__zzz__", onClick: function() { console.log("点击zzz") } },
                        ]
                    }

                    // 操作按钮
                    RowLayout {
                        Layout.alignment: Qt.AlignVCenter
                        spacing: 10
                        MusicActionButton { // 喜欢
                            url: "qrc:/icons/like.svg"
                        }
                        MusicActionButton { // 评论
                            url: "qrc:/icons/message.svg"
                        }
                        MusicActionButton { // 下载
                            url: "qrc:/icons/download.svg"
                        }
                        MusicActionButton { // 分享
                            url: "qrc:/icons/share.svg"
                        }
                    }
                }
            }

            // 居中
            ColumnLayout {
                id: centerLayout
                Layout.fillWidth: true
                Layout.preferredWidth: 400
                Layout.alignment: Qt.AlignVCenter
                spacing: 6

                // 歌曲操作按钮 @todo: 大小需要调整
                RowLayout {
                    Layout.fillWidth: true
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
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter
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
                    MusicProgressBar {
                        Layout.fillWidth: true
                    }
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
                Layout.minimumWidth: 200
                Layout.alignment: Qt.AlignRight

                // 占位
                Item {
                    Layout.fillWidth: true
                }

                // 播放序列
                PlayModeButton {
                }

                // 音量
                VolumeButton {
                }
                
                // 歌词
                Item {
                    Layout.preferredWidth: 32
                    Layout.preferredHeight: 32
                    MusicActionButton {
                        anchors.fill: parent
                        url: "qrc:/icons/lyrics.svg"
                        onClicked: lyricsState.switchWindow()
                    }
                }
            }
        }
    }
}
