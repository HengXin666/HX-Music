#include <widget/MusicTreeWidget.h>

#include <QMenu>
#include <QHeaderView>

#include <taglib/fileref.h>
#include <taglib/tag.h>

#include <utils/MusicInfo.hpp>
#include <singleton/SignalBusSingleton.h>
#include <cmd/MusicCommand.hpp>

MusicTreeWidget::MusicTreeWidget(QWidget* parent)
    : QTreeWidget(parent)
{
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::InternalMove);

    setHeaderLabels({
        "",
        "名称",
        "歌手",
        "专辑",
        "时长",
        "大小"
    });

    header()->setSectionResizeMode(QHeaderView::ResizeToContents); // 自动调整列宽

    // 双击节点
    connect(this, &QTreeWidget::itemActivated, this, 
        [this](QTreeWidgetItem* item, int column) {
        if (getNodeType(item) == NodeType::Folder) {
            // 文件夹操作
            qDebug() << "双击的是文件夹：" << item->text(0);
        } else {
            // 播放音乐
            SignalBusSingleton::get().newSongLoaded(
                HX::MusicInfo{QFileInfo{
                    item->data(
                        static_cast<int>(ItemData::FilePath), 
                        Qt::UserRole
                    ).toString()
                }}
            );
            MusicCommand::resume();
        }
    });

    // 展开节点
    connect(this, &QTreeWidget::itemExpanded, this, 
        [this](QTreeWidgetItem *item) {
        item->setIcon(1, QIcon{":/icons/folder-open.svg"});
    });

    // 关闭节点
    connect(this, &QTreeWidget::itemCollapsed, this, 
        [this](QTreeWidgetItem *item) {
        item->setIcon(1, QIcon{":/icons/folder-close.svg"});
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

    QByteArray fileName = QFile::encodeName( fileInfo.canonicalFilePath() );
    const char* encodedName = fileName.constData();
    TagLib::FileRef musicFile{encodedName};

    HX::MusicInfo musicInfo{fileInfo};
    item->setText(1, musicInfo.getTitle());
    item->setText(2, musicInfo.getArtist());
    item->setText(3, musicInfo.getAlbum());
    item->setText(4, musicInfo.formatTimeLengthToHHMMSS());
    item->setText(5, HX::FileInfo::convertByteSizeToHumanReadable(fileInfo.size()));
    // 根据需要设置图标，这里假设你已经有相应的图标资源
    if (auto img = musicInfo.getAlbumArtAdvanced()) {
        item->setIcon(1, QIcon{*img});
    } else {
        item->setIcon(1, QIcon{":/icons/audio.svg"});
    }
    
    item->setData(
        static_cast<int>(ItemData::FilePath),
        Qt::UserRole,
        fileInfo.filePath()
    );
    
    setNodeType(item, NodeType::File);

    // 如果拖拽到某个文件夹项上，可以进行判断并添加为其子项
    if (parentItem && getNodeType(parentItem) == NodeType::Folder) {
        parentItem->insertChild(index, item);
        parentItem->setExpanded(true);
    } else {
        insertTopLevelItem(index, item);
    }
    return true;
}

void MusicTreeWidget::addFolderItem(const QFileInfo &fileInfo, int index, QTreeWidgetItem *parentItem)  {
    namespace fs = std::filesystem;

    // 1. 新建一个目录元素
    QTreeWidgetItem* item = new QTreeWidgetItem;
    item->setIcon(1, QIcon{":/icons/folder-open.svg"});
    item->setText(1, fileInfo.fileName());
    setNodeType(item, NodeType::Folder);

    item->setData(
        static_cast<int>(ItemData::FilePath),
        Qt::UserRole,
        fileInfo.filePath()
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
                child->setText(0, QString("%1.").arg(++fileCnt));
            }
        }
        return;
    }
    for (int i = 0; i < parentItem->childCount(); ++i) {
        auto* child = parentItem->child(i);
        if (getNodeType(child) == NodeType::File) {
            child->setText(0, QString("%1.").arg(++fileCnt));
        }
    }
}
