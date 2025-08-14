pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Window
import HX.Music
import "./internal"

ProgressBarRect {
    id: root

    property int itemHeight: 100
    height: itemHeight

    color: "#88000000"

    RowLayout {
        anchors.fill: parent
        Layout.margins: 10
        spacing: 20

        // 靠左
        RowLayout {
            id: leftLayout
            Layout.preferredWidth: parent.width / 3
            Layout.alignment: Qt.AlignVCenter

            // 封面
            Item {
                id: cover
                Layout.preferredWidth: 64
                Layout.preferredHeight: 64
                Layout.margins: 3
                property string url: "qrc:/icons/audio.svg"

                MusicActionButton {
                    anchors.fill: parent
                    url: "qrc:/icons/audio.svg"
                    visible: cover.url == "qrc:/icons/audio.svg"
                }

                Image {
                    source: cover.url
                    visible: cover.url != "qrc:/icons/audio.svg"

                    anchors.fill: parent
                    anchors.centerIn: parent
                    fillMode: Image.PreserveAspectFit
                }
            }

            ColumnLayout {
                Layout.alignment: Qt.AlignVCenter

                // 滚动文本: 歌曲名 - 歌手... (可点击)
                LoopingScrollingText {
                    id: title
                    Layout.fillWidth: true
                    itemData: [
                        {
                            text: "HX-Music",
                            onClick: function () {
                                console.log("HX-Music: Hi");
                            }
                        }
                    ]
                }

                // 操作按钮 @todo: 大小需要调整
                RowLayout {
                    Layout.alignment: Qt.AlignVCenter
                    id: actionButtons
                    property int actionButtonSize: 24
                    spacing: 10
                    MusicActionButton {
                        Layout.preferredWidth: actionButtons.actionButtonSize
                        Layout.preferredHeight: actionButtons.actionButtonSize
                        // 喜欢
                        url: "qrc:/icons/like.svg"
                    }
                    MusicActionButton {
                        Layout.preferredWidth: actionButtons.actionButtonSize
                        Layout.preferredHeight: actionButtons.actionButtonSize
                        // 评论
                        url: "qrc:/icons/message.svg"
                    }
                    MusicActionButton {
                        Layout.preferredWidth: actionButtons.actionButtonSize
                        Layout.preferredHeight: actionButtons.actionButtonSize
                        // 下载
                        url: "qrc:/icons/download.svg"
                    }
                    MusicActionButton {
                        Layout.preferredWidth: actionButtons.actionButtonSize
                        Layout.preferredHeight: actionButtons.actionButtonSize
                        // 分享
                        url: "qrc:/icons/share.svg"
                    }

                    // 时间 xx:xx / yy:yy
                    RowLayout {
                        id: musicProgressBarLayout
                        Layout.alignment: Qt.AlignVCenter
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
                            color: Theme.paratextColor
                            Connections {
                                target: SignalBusSingleton
                                // 绑定信号: 播放位置变化
                                function onMusicPlayPosChanged(pos: int) {
                                    musicNowPosText.text = musicProgressBarLayout.formatTime(pos / 1000);
                                }
                            }
                        }
                        Text {
                            text: "/"
                            color: Theme.paratextColor
                        }
                        Text {
                            id: musicLengthText
                            text: "--:--"
                            color: Theme.paratextColor
                        }
                    }
                }
            }
        }

        // 居中
        // 歌曲操作按钮 @todo: 大小需要调整
        RowLayout {
            id: songActionButton
            Layout.preferredWidth: parent.width / 3
            property int actionButtonSize: 42
            property int actionButtonMiniSize: actionButtonSize * 0.666
            
            Layout.alignment: Qt.AlignHCenter
            spacing: 24

            Item {
                Layout.fillWidth: true
            }

            MusicActionButton {
                Layout.preferredWidth: songActionButton.actionButtonMiniSize
                Layout.preferredHeight: songActionButton.actionButtonMiniSize
                defaultColor: Theme.textColor
                pressedColor: Theme.textColor
                hoveredColor: Theme.highlightingColor
                url: "qrc:/icons/previous.svg"
                onClicked: MusicController.prev()
            }
            MusicActionButton {
                Layout.preferredWidth: songActionButton.actionButtonSize
                Layout.preferredHeight: songActionButton.actionButtonSize
                defaultColor: Theme.textColor
                pressedColor: Theme.textColor
                hoveredColor: Theme.highlightingColor
                url: MusicController.isPlaying ? "qrc:/icons/pause.svg" : "qrc:/icons/play.svg"
                onClicked: MusicController.togglePause()
            }
            MusicActionButton {
                Layout.preferredWidth: songActionButton.actionButtonMiniSize
                Layout.preferredHeight: songActionButton.actionButtonMiniSize
                defaultColor: Theme.textColor
                pressedColor: Theme.textColor
                hoveredColor: Theme.highlightingColor
                url: "qrc:/icons/next.svg"
                onClicked: MusicController.next()
            }

            Item {
                Layout.fillWidth: true
            }
        }

        // 靠右
        RowLayout {
            id: rightLayout
            Layout.preferredWidth: parent.width / 3
            Layout.alignment: Qt.AlignRight

            // 占位
            Item {
                Layout.fillWidth: true
            }

            // 播放序列
            PlayModeButton {}

            // 音量
            VolumeButton {}

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

    Connections {
        id: songConn
        target: null

        // 绑定信号: 更新歌曲
        function onNewSongLoaded(song: MusicInfo) {
            // 更新: 歌曲名 - 歌手
            const musicTitle = song.getTitle();
            const artists = song.getArtistList();
            const dataList = [
                {
                    text: musicTitle,
                    onClick: function () {
                        console.log(`点击了: ${musicTitle}`);
                    }
                }
            ];

            if (artists.length > 0) {
                dataList.push({
                    text: " - ",
                    onClick: () => {}
                });
            }
            let isBegin = true;
            for (const name of artists) {
                if (isBegin) {
                    isBegin = false;
                } else {
                    dataList.push({
                        text: "、",
                        onClick: () => {}
                    });
                }
                dataList.push({
                    text: name,
                    onClick: function () {
                        console.log(`点击了: ${name}`);
                    }
                });
            }
            title.itemData = dataList;

            // 更新封面
            cover.url = `image://imgPool/${song.filePath()}`;

            // 更新时长
            musicLengthText.text = musicProgressBarLayout.formatTime(song.getLengthInMilliseconds() / 1000);

            // 更新滚动播放状态, 需要延迟一步, 否则有bug, 内部还没有来得及更新
            Qt.callLater(() => {
                title.checkWidth(); // 外部长度没有刷新
            });
        }
    }

    Component.onCompleted: {
        songConn.target = SignalBusSingleton;
    }
}
