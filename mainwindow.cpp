#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QListView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QVBoxLayout>

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
    ui->_sidebar->setStyleSheet("QListView { background: #333; color: white; }"
                                "QListView::item { padding: 10px; }"
                                "QListView::item:selected { background: #555; }");

    // 创建模型
    QStandardItemModel *model = new QStandardItemModel(this);

    // 添加侧边栏菜单项
    QStringList menuItems = { "音乐", "听书", "直播", "我的收藏", "最近播放", "本地与下载" };
    for (const QString &text : menuItems) {
        QStandardItem *item = new QStandardItem(text);
        item->setIcon(QIcon(":/icons/music.png"));  // 这里需要替换为实际图标路径
        model->appendRow(item);
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

