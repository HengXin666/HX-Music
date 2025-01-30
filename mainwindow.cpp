#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QListView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QVBoxLayout>
#include <QPainter>
#include <QStyledItemDelegate>

// 自定义委托: 为 "分割线" 进行绘制
class SidebarDelegate : public QStyledItemDelegate {
public:
    explicit SidebarDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {}

    void paint(
        QPainter *painter,
        const QStyleOptionViewItem &option,
        const QModelIndex &index
    ) const override {
        QString text = index.data(Qt::DisplayRole).toString();

        // 如果是 "—— 分割线 ——", 就绘制线条
        if (text == "SEPARATOR") {
            painter->save();
            painter->setPen(QPen(Qt::gray, 1)); // 细灰色线条
            int y = option.rect.center().y();   // 水平居中
            painter->drawLine(option.rect.left() + 10, y, option.rect.right() - 10, y);
            painter->restore();
            return;
        }

        // 其它情况, 正常绘制文本 & 图标
        QStyledItemDelegate::paint(painter, option, index);
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 设置无边框窗口
    setWindowFlags(Qt::FramelessWindowHint);

    // 设置窗口只显示控件 (没有窗口体) (非常适合字幕!)
    // setAttribute(Qt::WA_TranslucentBackground);

    setWindowTitle("HX Music");

    resize(800, 600);

    // 设置透明度
    setWindowOpacity(0.80);

    // 初始化 左侧栏
    // ui->_sidebar->setStyleSheet(R"(
    //     QListView { background: #333; color: white; }
    //     QListView::item { padding: 10px; }
    //     QListView::item:selected { background: #555; }
    // )");

    // 绑定自定义绘制
    ui->_sidebar->setItemDelegate(new SidebarDelegate(ui->_sidebar));

    // 设置图标大小
    ui->_sidebar->setIconSize(QSize(24, 24));  // 统一图标尺寸

    // 创建模型
    QStandardItemModel *model = new QStandardItemModel(ui->_sidebar);

    // 左侧菜单项
    struct MenuItem {
        QString icon;
        QString text;
    };
    QList<MenuItem> items = {
        {"home", "音乐"},
        {"", "SEPARATOR"}, // 这里是分割线
        {"like", "我的收藏"},
        {"time", "最近播放"},
        {"download", "本地与下载"},
    };

    for (const auto &item : items) {
        QStandardItem *entry = new QStandardItem(
            QIcon(QString(":/icons/%1.svg").arg(item.icon)),
            item.text
        );
        entry->setSizeHint(QSize(100, 40)); // 控制行高
        entry->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        // 如果是分割线项，禁用它 & 设定特殊标识
        if (item.text == "SEPARATOR") {
            entry->setFlags(Qt::NoItemFlags);
        }

        model->appendRow(entry);
    }

    // 绑定模型
    ui->_sidebar->setModel(model);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_exitWindowBtn_clicked()
{
    this->close();
}


void MainWindow::on_toggleMaximizedBtn_clicked()
{
    if (isMaximized()) {
        showNormal();  // 恢复正常大小
    } else {
        showMaximized();  // 最大化
    }
}


void MainWindow::on_showMinimizBtn_clicked()
{
    showMinimized();
}

