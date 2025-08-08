import QtQuick

QtObject {
    id: theme

    // -----------------------------
    // 颜色系统(粉色主题)
    // -----------------------------

    // 背景颜色(主背景)
    readonly property color backgroundColor: "#4d4d4d"

    // 表面颜色(卡片/弹窗等区域)
    readonly property color surfaceColor: "#7a7a7a"

    // 文字颜色
    readonly property color textColor: "#3A0A2A"

    // 次要文字颜色(说明/占位等)
    readonly property color secondaryTextColor: "#8C3A60"

    // 边框颜色
    readonly property color borderColor: "#FFB3CF"

    // 强调色(主题主色, 如按钮、图标)
    readonly property color accentColor: "#FF69B4"
    // 错误颜色
    readonly property color errorColor: "#FF4C4C"

    // 禁用颜色
    readonly property color disabledColor: "#D8A7B1"

    // Hover/Active 背景
    readonly property color hoverColor: "#FFE0ED"
    readonly property color activeColor: "#FFCCE0"

    // -----------------------------
    // 字体系统
    // -----------------------------

    // 字体名称
    readonly property string fontFamily: "Inter"

    // 字体大小 (单位: 像素)
    readonly property int fontSizeSmall: 12     // 小字体, 例如提示信息
    readonly property int fontSizeNormal: 14    // 默认正文大小
    readonly property int fontSizeLarge: 18     // 标题或强调
    readonly property int fontSizeXL: 24        // 大标题
}
