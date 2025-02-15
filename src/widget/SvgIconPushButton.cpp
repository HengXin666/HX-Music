#include <widget/SvgIconPushButton.h>

#include <utils/SvgPars.hpp>

SvgIconPushButton::SvgIconPushButton(
    const QString& svgPath,
    const QColor& ordinary,
    const QColor& hover,
    QWidget* parent
)
    : QPushButton(parent)
    , _svgPath(svgPath)
    , _ordinaryIcon(
        HX::SvgPars{_svgPath}
            .replaceTagAndAttributeAndVal(
                "path",
                "fill",
                ordinary.name())
            .makeIcon())
    , _hoverIcon(
        HX::SvgPars{_svgPath}
            .replaceTagAndAttributeAndVal(
                "path",
                "fill",
                hover.name())
            .makeIcon())
{
    // 按下
    connect(this, &QPushButton::pressed, this, [this]{
        showHoverIcon();
    });

    // 弹起
    connect(this, &QPushButton::released, this, [this]{
        showOrdinaryIcon();
    });

    showOrdinaryIcon();
}

SvgIconPushButton* SvgIconPushButton::setOrdinaryIconColor(QColor const& color) {
    _ordinaryIcon
        = HX::SvgPars{_svgPath}
            .replaceTagAndAttributeAndVal(
                "path",
                "fill",
                color.name())
            .makeIcon();
    return this;
}

SvgIconPushButton* SvgIconPushButton::setHoverIconColor(QColor const& color) {
    _hoverIcon
        = HX::SvgPars{_svgPath}
            .replaceTagAndAttributeAndVal(
                "path",
                "fill",
                color.name())
            .makeIcon();
    return this;
}