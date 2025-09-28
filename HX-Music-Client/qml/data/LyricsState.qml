import QtQuick

QtObject {
    id: lyricsState

    property var componentLyrics: null    // 存储 Component
    property var windowLyrics: null       // 存储 实际创建的 Window 实例

    // @todo 应该修改为直接释放, 以便可以附着在活动窗口
    function switchWindow() {
        if (windowLyrics === null) {
            if (componentLyrics === null) {
                componentLyrics = Qt.createComponent("qrc:/LyricsWindow.qml");
            }
            // 加载
            if (componentLyrics.status === Component.Ready) {
                windowLyrics = componentLyrics.createObject(); // 指定父对象, 方便生命周期结束顺便带走子对象
                windowLyrics.reqClose.connect(() => switchWindow());
                if (windowLyrics !== null) {
                    windowLyrics.show();
                    LyricController.isWindowOpened = true;
                } else {
                    console.error("创建窗口失败");
                }
            } else {
                console.error("组件加载失败:", componentLyrics.errorString());
            }
        } else {
            // 切换窗口显示/隐藏
            if (windowLyrics.visible) {
                if (windowLyrics.locked) {
                    windowLyrics.unlock();
                } else {
                    LyricController.isWindowOpened = false;
                    windowLyrics.hide();
                }
            } else {
                windowLyrics.unlock();
                windowLyrics.show();
                LyricController.isWindowOpened = true;
            }
        }
    }

    function del() {
        if (windowLyrics !== null) {
            if (typeof windowLyrics.destroy === "function") {
                windowLyrics.destroy();
            }
            windowLyrics = null;
        }

        if (componentLyrics !== null) {
            if (typeof componentLyrics.destroy === "function") {
                componentLyrics.destroy();
            }
            componentLyrics = null;
        }
    }

    Component.onCompleted: {
        Qt.callLater(() => {
            if (LyricController.isWindowOpened) {
                switchWindow();
                if (LyricController.isLocked) {
                    windowLyrics.lock();
                }
            }
        });
    }
}
