#include <widget/Sidebar.h>

#include <QPushButton>

Sidebar::Sidebar(QWidget* parent)
    : QListWidget(parent)
{
    /* 侧边栏 ui
        音乐
        ---
        我的收藏
        最近播放
        本地与下载
        ---
        我的歌单
        ...
    */
    QString divider{"-----"};

    for (auto& it : {
        new QListWidgetItem {QIcon{":/icons/home.svg"}, "音乐"},
        new QListWidgetItem {divider},
        new QListWidgetItem {QIcon{":/icons/like.svg"}, "我的收藏"},
        new QListWidgetItem {QIcon{":/icons/time.svg"}, "最近播放"},
        new QListWidgetItem {QIcon{":/icons/download.svg"}, "本地与下载"},
        new QListWidgetItem {divider},
    }) {
        addItem(it);
    }

    // 连接 currentRowChanged 信号到槽, 获取点击的项的索引
    connect(this, &QListWidget::currentRowChanged, this,
    [this, divider = std::move(divider)](
        int index
    ) {
        auto* item = this->item(index);
        if (item->text() == divider) // 屏蔽分割线的影响
            return;
        qDebug() << index;
    });
}