pragma ComponentBehavior: Bound
import QtQuick 2.15

Item {
    id: root
    width: 24
    height: 24

    property int barCount: 5
    property bool playing: true
    property real barWidth: 2.5
    property real barSpacing: 2
    property int maxBarHeight: 24
    property int minBarHeight: 3
    property color color: "#1DB954"

    Row {
        id: barsRow
        anchors.centerIn: parent
        spacing: root.barSpacing

        Repeater {
            model: root.barCount

            Item {
                width: root.barWidth
                height: root.maxBarHeight // 固定容器高度，防止Row整体跳动

                Rectangle {
                    id: bar
                    width: parent.width
                    height: root.minBarHeight
                    radius: width / 2
                    color: root.color
                    anchors.bottom: parent.bottom

                    property real targetHeight: root.minBarHeight

                    Behavior on height {
                        NumberAnimation {
                            duration: 200
                            easing.bezierCurve: [0.25, 0.1, 0.25, 1.0]
                        }
                    }

                    Timer {
                        id: bounceTimer
                        interval: 200 + Math.floor(Math.random() * 60) // 打乱间隔
                        running: root.playing
                        repeat: true
                        onTriggered: {
                            const values = [0, 0.2, 0.5, 0.7, 1];
                            const r = values[Math.floor(Math.random() * values.length)];
                            bar.targetHeight = r * (root.maxBarHeight - root.minBarHeight) + root.minBarHeight;
                            bar.height = bar.targetHeight;
                        }
                    }

                    Component.onCompleted: {
                        if (root.playing)
                            bounceTimer.start();
                    }

                    Connections {
                        target: root
                        function onPlayingChanged() {
                            if (root.playing)
                                bounceTimer.start();
                            else
                                bounceTimer.stop();
                        }
                    }
                }
            }
        }
    }
}
