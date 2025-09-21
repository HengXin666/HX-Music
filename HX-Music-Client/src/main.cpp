#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <singleton/SignalBusSingleton.h>
#include <singleton/GlobalSingleton.hpp>
#include <singleton/OnlineImagePoll.h>
#include <singleton/NetImagePoll.h>
#include <controller/LyricController.h>
#include <controller/MusicController.h>
#include <controller/PlaylistController.h>
#include <controller/MessageController.h>
#include <controller/UserController.h>
#include <cmd/MusicCommand.hpp>
#include <config/MusicConfig.hpp>
#include <config/Theme.hpp>
#include <utils/SvgPars.hpp>
#include <utils/WindowMaskUtil.h>
#include <model/MusicListModel.hpp>
#include <model/AllMusicListModel.hpp>
#include <model/PlaylistModel.hpp>
#include <model/UploadListModel.hpp>

namespace HX {

/**
 * @brief 客户端
 */
struct MusicClient {
    MusicClient(int argc, char* argv[])
        : _app{argc, argv}
        , _engine{}
    {
        QObject::connect(
            &_engine,
            &QQmlApplicationEngine::objectCreationFailed,
            &_app,
            []() { QCoreApplication::exit(-1); },
            Qt::QueuedConnection
        );
    }

    MusicClient& loadConfig() {
        return *this;
    }

    MusicClient& buildQml() {
        // 注册枚举
        qmlRegisterUncreatableMetaObject(
            HX::PlayListTypeWrapper::staticMetaObject,
            "HX.Music", // QML import 名称
            1, 0,
            "PlayMode", // QML 中使用名
            "PlayMode 是一个枚举类型, 不可实例化"
        );

        QQmlContext* cp = _engine.rootContext();
        // 信号总线
        cp->setContextProperty("SignalBusSingleton", &HX::SignalBusSingleton::get());

        // 窗口掩码工具类
        cp->setContextProperty("WindowMaskUtil", &_windowMaskUtil);

        // 用户控制类
        static HX::UserController userController{};
        cp->setContextProperty("UserController", &userController);

        // 主题数据类
        static HX::Theme theme{};
        cp->setContextProperty("Theme", &theme);

        // 歌单控制类
        cp->setContextProperty("PlaylistController", &HX::PlaylistController::get());

        // 音乐控制类
        static HX::MusicController musicController{};
        cp->setContextProperty("MusicController", &musicController);

        // 注册 歌单列表视图 到 qml
        qmlRegisterType<HX::PlaylistModel>(
            "HX.Music",
            1, 0,
            "PlaylistModel"
        );

        // 注册 全部歌曲列表视图 到 qml
        qmlRegisterType<HX::AllMusicListModel>(
            "HX.Music",
            1, 0,
            "AllMusicListModel"
        );

        // 注册 歌曲列表视图 到 qml
        qmlRegisterType<HX::MusicListModel>(
            "HX.Music",
            1, 0,
            "MusicListModel"
        );

        // 注册 上传列表视图 到 qml
        qmlRegisterType<HX::UploadListModel>(
            "HX.Music",
            1, 0,
            "UploadListModel"
        );

        // 注册 音频信息
        qmlRegisterType<HX::MusicInformation>(
            "HX.Music",
            1, 0,
            "MusicInformation"
        );

        // 注册消息控制器
        qmlRegisterSingletonInstance(
            "HX.Music",
            1, 0,
            "MessageController",
            &HX::MessageController::get()
        );

        // 注册用于添加到歌单的菜单项的数据
        qmlRegisterUncreatableType<HX::PlaylistInfoData>(
            "HX.Music",
            1, 0,
            "PlaylistInfo",
            "PlaylistInfo is a data type"
        );

        // 注册 音频信息
        qRegisterMetaType<HX::MusicInformation>("MusicInformation");

        // 歌词渲染与管理
        auto* lyricController = new HX::LyricController;
        cp->setContextProperty("LyricController", lyricController);
        // 注册img链接: image://musicLyric
        _engine.addImageProvider("musicLyric", lyricController); // 内部会释放!
        
        // 注册img链接: image://svgColored
        _engine.addImageProvider("svgColored", new HX::QmlSvgPars); // 内部会释放!

        // 注册img链接: image://onlineImagePoll
        _engine.addImageProvider("onlineImagePoll", HX::OnlineImagePoll::get()); // 内部会释放!

        // 注册img链接: image://netImagePoll
        _engine.addImageProvider("netImagePoll", new HX::NetImagePoll{}); // 内部会释放!

        // 应该使用 _ 和 [0-9a-Z], 不能使用`-`
        _engine.loadFromModule("HX.Music", "Main");

        // 集中保存
        QObject::connect(&_app, &QCoreApplication::aboutToQuit, [&, lyricController] {
            // 配置保存
            HX::GlobalSingleton::get().musicConfig.position = musicController.getNowPos();
            HX::GlobalSingleton::get().saveConfig();
            lyricController->saveConfig();
        });
        return *this;
    }

    int exec() {
        return _app.exec();
    }

    MusicClient& operator=(MusicClient&&) noexcept = delete;
    
private:
    QGuiApplication _app;
    QQmlApplicationEngine _engine;
    HX::WindowMaskUtil _windowMaskUtil{};
};

} // namespace HX


int main(int argc, char* argv[]) {
    HX::MusicClient app{argc, argv};
    return app.loadConfig()
              .buildQml()
              .exec();
}