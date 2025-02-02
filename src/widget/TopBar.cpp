#include <widget/TopBar.h>

#include <QHBoxLayout>
#include <QMenu>

TopBar::TopBar(QWidget* parent)
    : QWidget(parent)
{
    /* 顶部栏 ui
        搜索栏 | ... | 头像 用户名 等级 | 消息 更多(设置/...反正是个选项卡) | 隐藏 最大化 关闭
    */
    QHBoxLayout* hBL = new QHBoxLayout(this);

    // 搜索栏
    QHBoxLayout* hlSearch = new QHBoxLayout(this);
    _btnSearch->setIcon(QIcon{":/icons/search.svg"});
    _textSearch->setPlaceholderText("搜索");
    hlSearch->addWidget(_btnSearch);
    hlSearch->addWidget(_textSearch);
    hBL->addLayout(hlSearch);
    hBL->addStretch();

    // 头像 用户名 等级
    QHBoxLayout* hlUser = new QHBoxLayout(this);
    _imgAvatar->setText("头像");
    _textUsername->setText("用户名");
    _textLevel->setText("Lv 6");
    hlUser->addWidget(_imgAvatar);
    hlUser->addWidget(_textUsername);
    hlUser->addWidget(_textLevel);
    hBL->addLayout(hlUser);

    // 消息 更多(设置/...反正是个选项卡)
    _btnMsg->setIcon(QIcon{":/icons/mail.svg"});
    _toolBtn->setIcon(QIcon{":/icons/more.svg"});
    _toolBtn->setPopupMode(QToolButton::InstantPopup);
    //创建菜单
    QMenu * menu = new QMenu();
    menu->addAction(QIcon{":/icons/setting.svg"}, "设置");
    menu->addSeparator();
    menu->addAction(QIcon{":/icons/tip.svg"}, "关于");
    _toolBtn->setMenu(menu);
    hBL->addWidget(_btnMsg);
    hBL->addWidget(_toolBtn);

    auto* separator = new QLabel(this);
    separator->setText("|");
    hBL->addWidget(separator);

    // 隐藏 最大化 关闭
    QHBoxLayout* hlWindow = new QHBoxLayout(this);
    _btnHide->setIcon(QIcon{":/icons/dropdown.svg"});
    _btnMaximize->setIcon(QIcon{":/icons/up.svg"});
    _btnClose->setIcon(QIcon{":/icons/close.svg"});
    // 监听主窗口状态变化
    QWidget *mainWindow = this->window();  // 获取主窗口
    if (mainWindow) {
        mainWindow->installEventFilter(this);
    }
    connect(_btnHide, &QPushButton::clicked, this, [this, mainWindow]() {
        mainWindow->showMinimized();
    });
    connect(_btnMaximize, &QPushButton::clicked, this, [this]() {
        toggleMaximize();
    });
    connect(_btnClose, &QPushButton::clicked, this, [this, mainWindow]() {
        mainWindow->close();
    });
    hlWindow->addWidget(_btnHide);
    hlWindow->addWidget(_btnMaximize);
    hlWindow->addWidget(_btnClose);
    hBL->addLayout(hlWindow);

    setLayout(hBL);
}