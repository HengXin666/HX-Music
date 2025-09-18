import QtQuick

Rectangle {
    id: root
    width: 60
    height: 30
    color: "transparent"

    // 外部可传属性
    property alias text: label.text
    property alias textColor: label.color
    property color highlightColor: "blue"
    property bool checked: false
    signal clicked()

    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked()
    }

    // 按钮文字
    Text {
        id: label
        anchors.centerIn: parent
        color: "white"
        font.pixelSize: 14
    }

    // 右边竖线
    Canvas {
        id: lineCanvas
        anchors.fill: parent
        onPaint: {
            const ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);
            if (!root.checked) {
                return;
            }
            ctx.strokeStyle = root.highlightColor;
            ctx.lineWidth = 1;
            ctx.beginPath();
            ctx.moveTo(width - 1, 0);
            ctx.lineTo(width - 1, height);
            ctx.stroke();
        }
    }

    onHighlightColorChanged: lineCanvas.requestPaint()
    onCheckedChanged: lineCanvas.requestPaint()
}
