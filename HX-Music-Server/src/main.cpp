#include <HXLibs/log/Log.hpp>

auto hx_init = []{
    setlocale(LC_ALL, "zh_CN.UTF-8");
    using namespace HX;
    try {
        auto cwd = std::filesystem::current_path();
        log::hxLog.debug("当前工作路径是:", cwd);
        std::filesystem::current_path("../../data");
        log::hxLog.debug("切换到路径:", std::filesystem::current_path());
    } catch (const std::filesystem::filesystem_error& e) {
        log::hxLog.error("Error:", e.what());
    }
    return 0;
}();

#include <db/SQLiteMeta.hpp>
#include <HXLibs/reflection/json/JsonRead.hpp>
#include <HXLibs/reflection/json/JsonWrite.hpp>

using namespace HX;

namespace HX::db {

template <typename U>
struct SQLiteSqlType<std::vector<U>> {
    using T = std::vector<U>;
    static constexpr std::string bind(T const& t) noexcept {
        std::string res;
        reflection::toJson(t, res);
        return res;
    }

    static constexpr T columnType(std::string_view str) {
        T t{};
        reflection::fromJson(t, str);
        return t;
    }
};

template <typename T>
    requires (std::is_enum_v<T>)
struct SQLiteSqlType<T> {
    static constexpr std::string bind(T const& t) noexcept {
        std::string res;
        reflection::toJson(t, res);
        return res;
    }

    static constexpr T columnType(std::string_view str) {
        T t{};
        reflection::fromJson(t, str);
        return t;
    }
};

} // namespace HX::db

#include <api/MusicApi.hpp>
#include <api/PlaylistApi.hpp>
#include <api/CoverApi.hpp>
#include <api/LyricsApi.hpp>
#include <api/UserApi.hpp>

#include <filesystem>

void ininDir() {
    // 创建文件夹
    std::filesystem::create_directories("file/music");
    std::filesystem::create_directories("file/db");
    std::filesystem::create_directories("file/cover");
    std::filesystem::create_directories("file/lyrics");
    std::filesystem::create_directories("file/lyrics/ass");

    // 初始化
    auto userDAO = dao::MemoryDAOPool::get<UserDAO, config::UserDbPath>();

    try {
        userDAO->at(1);
    } catch (...) {
        auto nacl = utils::Uuid::makeV4();
        userDAO->add<UserDO>({
            {},
            "hx",
            {},
            nacl,
            utils::md5("hx666" + nacl),
            {},
            {},
            PermissionEnum::Administrator
        });
    }

    // 测试数据
    [&] {
        auto playlistDAO 
            = dao::MemoryDAOPool::get<PlaylistDAO, config::PlaylistDbPath>();
        try {
            playlistDAO->update<true, PlaylistDO>({
                1, "HX-测试歌单", "测试没有描述...", {
                    1, 2, 3, 4, 5
                }
            });
        } catch (...) {
            playlistDAO->add<PlaylistDO>({
                1, "HX-测试歌单", "测试没有描述...", {
                    1, 2, 3, 4, 5
                }
            });
        }

        // auto musicDAO 
        //     = dao::MemoryDAOPool::get<MusicDAO, "./file/db/music.db">();
        // log::hxLog.info(musicDAO->at(1));
    }();
}

#include <csignal>

container::FutureResult<bool> isStop;

int main() {
    ininDir();
    net::HttpServer server{"0.0.0.0", "28205"};
    api::addApi<MusicApi>(server);
    api::addApi<PlaylistApi>(server);
    api::addApi<CoverApi>(server);
    api::addApi<LyricsApi>(server);
    api::addApi<UserApi>(server);

    std::signal(SIGINT, [](int s) {
        if (s == SIGINT) {
            isStop.getFutureResult()->setData(true);
        }
    });
    server.asyncRun<decltype(utils::operator""_s<"15">())>(16, {});
    isStop.wait();
    // 析构 pybind
    getToKaRaOKAssPtr()->release();
    exit(0);
    return 0;
}
