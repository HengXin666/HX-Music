#include <widget/MusicTreeWidget.h>

#include <QMenu>
#include <QHeaderView>

#include <taglib/fileref.h>
#include <taglib/tag.h>

#include <singleton/SignalBusSingleton.h>
#include <singleton/GlobalSingleton.hpp>
#include <utils/MusicInfo.hpp>
#include <cmd/MusicCommand.hpp>

void MultiLineItemDelegate::paint(
    QPainter* painter,
    const QStyleOptionViewItem& option,
    const QModelIndex& index
) const {
    if (index.column() == static_cast<int>(MusicTreeWidget::ItemData::Title)) {
        // 初始化基本样式 (处理选中状态等)
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        // 绘制选中状态的背景
        if (option.state & (QStyle::State_Selected | QStyle::State_MouseOver)) {
            painter->fillRect(option.rect, option.palette.highlight());
            painter->setPen(option.palette.highlightedText().color());
        } else {
            painter->setPen(option.palette.color(QPalette::Text));
        }

        /*
            -----  文本1
            |图片|
            -----  文本2
        */

        // 布局参数
        const QRect contentRect = opt.rect.adjusted(
            Padding, Padding, 
            -Padding, -Padding
        );

        // 绘制图片 (左对齐 + 垂直居中)
        QPixmap pixmap = qvariant_cast<QPixmap>(index.data(Qt::DecorationRole));
        QRect imageRect(
            contentRect.left(), 
            contentRect.top() + (contentRect.height() - ImageSize) / 2,  // 垂直居中
            ImageSize, 
            ImageSize
        );
        
        if (!pixmap.isNull()) {
            painter->drawPixmap(
                imageRect,
                pixmap.scaled(
                    ImageSize,
                    ImageSize, 
                    Qt::KeepAspectRatio,
                    Qt::SmoothTransformation
                )
            );
        }

        // 文本区域
        QRect textRect = contentRect;
        textRect.setTop(contentRect.top() + TextMargin); // 上边距
        textRect.setLeft(imageRect.right() + Padding);
        textRect.setRight(contentRect.right());

        // 分割文本
        QStringList lines = index.data(Qt::DisplayRole).toString().split("\n");
        if (lines.isEmpty()) 
            return;

        // 字体设置 (硬编码值)
        QFont titleFont = painter->font();
        titleFont.setPointSize(TitleFontSize);   // 名称字体
        QFont artistFont = titleFont;
        artistFont.setPointSize(ArtistFontSize); // 歌手字体

        // 绘制第一行 (名称)
        painter->setFont(titleFont);
        QRect titleRect = textRect;
        titleRect.setHeight(QFontMetrics(titleFont).height());
        painter->drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, lines[0]);

        // 绘制第二行 (歌手)
        if (lines.size() > 1) {
            painter->setFont(artistFont);
            QRect artistRect = textRect;

            // 计算下边距
            int h = QFontMetrics(artistFont).height();
            artistRect.setTop(contentRect.bottom() - h - TextMargin);
            artistRect.setBottom(contentRect.bottom() - TextMargin);

            artistRect.setHeight(h);
            painter->drawText(artistRect, Qt::AlignLeft | Qt::AlignVCenter, lines[1]);
        }
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

MusicTreeWidget::MusicTreeWidget(QWidget* parent)
    : QTreeWidget(parent)
{
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::InternalMove);

    setHeaderLabels({
        "歌曲",
        "专辑",
        "时长",
        ""     // 保留
    });

    // 禁止拖动项
    header()->setSectionsMovable(false);

    // 禁止拖动宽度
    header()->setSectionResizeMode(QHeaderView::Fixed);

    // 禁止显示横向滚动条
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 设置元素自定义绘制代理类
    auto* delegate = new MultiLineItemDelegate(this);
    setItemDelegate(delegate);

    // 设置视图的固定行高
    QStyleOptionViewItem option;
    QSize size = delegate->sizeHint(option, model()->index(0, 0));
    header()->setDefaultSectionSize(size.height());

    // 双击节点
    connect(this, &QTreeWidget::itemActivated, this, 
        [this](QTreeWidgetItem* item, int column) {
        if (getNodeType(item) == NodeType::Folder) {
            // 文件夹操作
            qDebug() << "双击的是文件夹：" << item->text(0);
        } else {
            // 播放音乐
            MusicCommand::selectMusic(
                item->data(
                    static_cast<int>(ItemData::PlayQueue),
                    Qt::UserRole
                ).value<HX::PlayQueue::iterator>()
            );
            MusicCommand::resume();
        }
    });

    // 展开节点
    connect(this, &QTreeWidget::itemExpanded, this, 
        [this](QTreeWidgetItem *item) {
        item->setData(static_cast<int>(ItemData::Title), Qt::DecorationRole, QPixmap{":/icons/folder-open.svg"});
    });

    // 关闭节点
    connect(this, &QTreeWidget::itemCollapsed, this, 
        [this](QTreeWidgetItem *item) {
        item->setData(static_cast<int>(ItemData::Title), Qt::DecorationRole, QPixmap{":/icons/folder-close.svg"});
    });

    // 在构造函数中设置上下文菜单策略并连接信号
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QTreeWidget::customContextMenuRequested, this,
        [this](const QPoint& pos) {
        // 根据点击位置获取对应的节点
        QTreeWidgetItem *item = itemAt(pos);
        if (!item)
            return;  // 如果没有点击到有效节点，可以直接返回

        // 创建菜单，并添加所需动作
        QMenu menu;
        QAction *openAction = menu.addAction("打开");
        QAction *deleteAction = menu.addAction("删除");
        QAction *propertiesAction = menu.addAction("属性");

        // 将本地坐标转换为全局坐标，显示菜单
        QAction *selectedAction = menu.exec(mapToGlobal(pos));

        if (selectedAction == openAction) {
            qDebug() << "选择了打开：" << item->text(0);
        } else if (selectedAction == deleteAction) {
            qDebug() << "选择了删除：" << item->text(0);
        } else if (selectedAction == propertiesAction) {
            qDebug() << "选择了属性：" << item->text(0);
        }
    });
}

bool MusicTreeWidget::addFileItem(
    const QFileInfo &fileInfo, 
    int index, 
    QTreeWidgetItem *parentItem
) {
    QTreeWidgetItem *item = new QTreeWidgetItem;
    if (HX::MusicInfo::isNotSupport(fileInfo)) {
        return false;
    }

    QByteArray fileName = QFile::encodeName(fileInfo.canonicalFilePath());
    TagLib::FileRef musicFile{fileName.constData()};

    HX::MusicInfo musicInfo{fileInfo};
    item->setData(
        static_cast<int>(ItemData::Title),
        Qt::DisplayRole,
        QString{"%1\n%2"}
            .arg(musicInfo.getTitle())
            .arg(musicInfo.getArtist())
    );

    item->setText(1, musicInfo.getAlbum());
    item->setText(2, musicInfo.formatTimeLengthToHHMMSS());
    item->setToolTip(2, HX::FileInfo::convertByteSizeToHumanReadable(fileInfo.size()));
    
    // 根据需要设置图标，这里假设你已经有相应的图标资源
    if (auto img = musicInfo.getAlbumArtAdvanced()) {
        item->setData(static_cast<int>(ItemData::Title), Qt::DecorationRole, *img);
    } else {
        item->setData(static_cast<int>(ItemData::Title), Qt::DecorationRole, QPixmap{":/icons/audio.svg"});
    }
    
    item->setData(
        static_cast<int>(ItemData::FilePath),
        Qt::UserRole,
        fileInfo.filePath()
    );
    
    setNodeType(item, NodeType::File);

    QVariant v{};
    v.setValue(GlobalSingleton::get().playQueue.insert(
        parentItem && getNodeType(parentItem) == NodeType::Folder
            ? HX::PlayQueue::ItOpt{parentItem->data(
                    static_cast<int>(ItemData::PlayQueue),
                    Qt::UserRole
                ).value<HX::PlayQueue::iterator>()}
            : HX::PlayQueue::ItOpt{}, 
        HX::PlayQueue::Node{fileInfo.canonicalFilePath()}
    ));
    item->setData(
        static_cast<int>(ItemData::PlayQueue),
        Qt::UserRole,
        v
    );

    // 如果拖拽到某个文件夹项上，可以进行判断并添加为其子项
    if (parentItem && getNodeType(parentItem) == NodeType::Folder) {
        parentItem->insertChild(index, item);
        parentItem->setExpanded(true);
    } else {
        insertTopLevelItem(index, item);
    }
    return true;
}

void MusicTreeWidget::addFolderItem(const QFileInfo &fileInfo, int index, QTreeWidgetItem* parentItem)  {
    namespace fs = std::filesystem;

    // 1. 新建一个目录元素
    QTreeWidgetItem* item = new QTreeWidgetItem;
    item->setData(static_cast<int>(ItemData::Title), Qt::DecorationRole, QPixmap{":/icons/folder-open.svg"});
    item->setData(static_cast<int>(ItemData::Title), Qt::DisplayRole, fileInfo.fileName());
    setNodeType(item, NodeType::Folder);

    item->setData(
        static_cast<int>(ItemData::FilePath),
        Qt::UserRole,
        fileInfo.filePath()
    );

    QVariant v{};
    v.setValue(GlobalSingleton::get().playQueue.insert(
        parentItem && getNodeType(parentItem) == NodeType::Folder
            ? HX::PlayQueue::ItOpt{parentItem->data(
                    static_cast<int>(ItemData::PlayQueue),
                    Qt::UserRole
                ).value<HX::PlayQueue::iterator>()}
            : HX::PlayQueue::ItOpt{}, 
        HX::PlayQueue::Node{HX::PlayQueue::List{}}
    ));
    item->setData(
        static_cast<int>(ItemData::PlayQueue),
        Qt::UserRole,
        v
    );

    if (parentItem && getNodeType(parentItem) == NodeType::Folder) {
        parentItem->insertChild(index, item);
        parentItem->setExpanded(true);
    } else {
        insertTopLevelItem(index, item);
    }

    // 2. 递归遍历文件夹, 并且构建
    int i = 0;
    for (auto const& it : fs::directory_iterator{
        fileInfo.filesystemFilePath()
    }) {
        if (it.is_directory()) { // 是文件夹
            addFolderItem(QFileInfo{it.path()}, i, item);
        } else {
            addFileItem(QFileInfo{it.path()}, i, item);
        }
        ++i;
    }

    // 3. 更新计数
    updateItemNumber(item);
}

void MusicTreeWidget::updateItemNumber(QTreeWidgetItem *parentItem) {
    int fileCnt = 0;
    if (!parentItem) {
        for (int i = 0; i < topLevelItemCount(); ++i) {
            auto* child = topLevelItem(i);
            if (getNodeType(child) == NodeType::File) {
                child->setData(static_cast<int>(ItemData::Title), Qt::UserRole, ++fileCnt);
            }
        }
        return;
    }
    for (int i = 0; i < parentItem->childCount(); ++i) {
        auto* child = parentItem->child(i);
        if (getNodeType(child) == NodeType::File) {
            child->setData(static_cast<int>(ItemData::Title), Qt::UserRole, ++fileCnt);
        }
    }
}
