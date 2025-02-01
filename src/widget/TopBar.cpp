#include <widget/TopBar.h>

#include <QHBoxLayout>

TopBar::TopBar(QWidget* parent)
    : QWidget(parent)
{
    /* 顶部栏 ui
        搜索栏 | ... | 头像 用户名 等级 消息 | 更多(设置/...反正是个选项卡) | 隐藏 最大化 关闭
    */
    QHBoxLayout* hBL = new QHBoxLayout(this);

    // 搜索栏
    QHBoxLayout* hlSearch = new QHBoxLayout(this);
    _btnSearch->setIcon(QIcon{":/icons/search.svg"});
    _textSearch->setPlaceholderText("搜索");
    hlSearch->addWidget(_btnSearch);
    hlSearch->addWidget(_textSearch);
    hBL->addLayout(hlSearch);

    setLayout(hBL);
}