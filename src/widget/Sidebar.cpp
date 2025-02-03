#include <widget/Sidebar.h>

#include <vector>
#include <tuple>
#include <functional>
#include <QPushButton>

#include <utils/TupleElementExtractor.hpp>
#include <widget/DividerWidget.h>
#include <singleton/GlobalSingleton.hpp>
#include <views/LocalAndDownloadView.h>

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
    QString divider{};

    std::vector<std::tuple<QListWidgetItem*, std::function<void()>>>
        itemFuns {
        {new QListWidgetItem {QIcon{
            ":/icons/home.svg"},
            "音乐"}, 
        []{}},
        {new QListWidgetItem {divider}, []{}},
        {new QListWidgetItem {QIcon{
            ":/icons/like.svg"},
            "我的收藏"},
        []{}},
        {new QListWidgetItem {QIcon{
            ":/icons/time.svg"},
            "最近播放"},
        []{}},
        {new QListWidgetItem {
            QIcon{":/icons/download.svg"},
            "本地与下载"},
        []{
            // 切换到 本地与下载 界面
            GlobalSingleton::get().imp.pushView(new LocalAndDownloadView);
        }},
        {new QListWidgetItem {divider}, []{}},
    };

    for (auto& it : HX::TupleElementExtractor::extractorToVector<0>(itemFuns)) {
        if (it->text() == divider) {
            QListWidgetItem* dividerItem = new QListWidgetItem();
            dividerItem->setFlags(Qt::NoItemFlags); // 禁用选择效果
            addItem(dividerItem);
            setItemWidget(dividerItem, new DividerWidget());
        } else {
            addItem(it);
        }
    }

    // 连接 currentRowChanged 信号到槽, 获取点击的项的索引
    connect(this, &QListWidget::currentRowChanged, this,
    [this, funs = HX::TupleElementExtractor::extractorToVector<1>(itemFuns)](
        int index
    ) {
        auto* item = this->item(index);
        qDebug() << index;
        funs[index]();
    });
}