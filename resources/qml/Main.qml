import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import HX.Music
import "./widget"
import "./data"
import "./window"

BorderlessWindow {
    id: mainWin
    minimumWidth: 1080
    minimumHeight: 720
    visible: true
    title: "HX.Music"
    showBorder: false

    // === 全局状态 ===
    property var lyricsState: LyricsState{} // 歌词悬浮窗口状态控制
    property var theme: ThemeData{}         // 全局主题配置
    property var musicController: MusicController {} // 音乐控制

    onClosing: {
        lyricsState.del();
        Qt.quit(); // 强制退出应用
    }

    delegate: Rectangle {
        anchors.fill: parent
        ColumnLayout {
            anchors.fill: parent

            // 多个标签页面
            RowLayout {
                SideBar {
                    itemWidth: 100
                    Layout.preferredWidth: 100
                    Layout.fillHeight: true
                    onTabClicked: (index) => {
                        console.log("点击了标签页:", index);
                        stackView.currentIndex = index;
                    }
                }

                StackLayout {
                    id: stackView
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Rectangle {
                        MusicListView {
                            anchors.fill: parent
                        }
                    }
                    Rectangle { color: "#d0eaff" }
                    Rectangle { color: "#cdeccd" }
                    Rectangle { color: "#ffe0b2" }
                }
            }

            // 音乐播放操作条
            PlaybackBar {
                itemHeight: 72
                Layout.fillWidth: true
            }
        }
    }
}
