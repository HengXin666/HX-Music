import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import HX.Music
import "./widget"
import "./data"

ApplicationWindow {
    id: mainWin
    minimumWidth: 640
    minimumHeight: 480

    visible: true
    title: "HX.Music"

    // === 全局状态 ===
    property var lyricsState: LyricsState{}

    onClosing: {
        lyricsState.del();
        Qt.quit(); // 强制退出应用
    }

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
