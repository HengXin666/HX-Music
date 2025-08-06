import QtQuick
import QtQuick.Controls
import QtQuick.Window

ApplicationWindow {
    id: mainWin
    width: 640
    height: 480
    visible: true
    title: "HX.Music"

    property var componentLyrics: null    // 存储 Component
    property var windowLyrics: null       // 存储 实际创建的 Window 实例

    Button {
        text: "歌词浮窗"
        anchors.centerIn: parent

        onClicked: {
            if (mainWin.windowLyrics === null) {
                if (mainWin.componentLyrics === null) {
                    mainWin.componentLyrics = Qt.createComponent("FloatingLyricsWindow.qml");
                }
                // 加载
                if (mainWin.componentLyrics.status === Component.Ready) {
                    mainWin.windowLyrics = mainWin.componentLyrics.createObject(mainWin); // 指定父对象, 方便生命周期结束顺便带走子对象
                    if (mainWin.windowLyrics !== null) {
                        mainWin.windowLyrics.show();
                    } else {
                        console.error("创建窗口失败");
                    }
                } else {
                    console.error("组件加载失败:", mainWin.componentLyrics.errorString());
                }
            } else {
                // 切换窗口显示/隐藏
                if (mainWin.windowLyrics.visible) {
                    mainWin.windowLyrics.hide();
                } else {
                    mainWin.windowLyrics.unlock();
                    mainWin.windowLyrics.show();
                }
            }
        }
    }
}
