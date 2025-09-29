#include <HXLibs/log/Log.hpp>

auto hx_init = []{
    setlocale(LC_ALL, "zh_CN.UTF-8");
    using namespace HX;
    try {
        auto cwd = std::filesystem::current_path();
        log::hxLog.info("当前工作路径是:", cwd);
        std::filesystem::current_path("../../data");
        log::hxLog.info("切换到路径:", std::filesystem::current_path());
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
    std::filesystem::create_directories("file/avatar");
    std::filesystem::create_directories("file/lyrics");
    std::filesystem::create_directories("file/lyrics/ass");

    // 初始化
    auto userDAO = dao::MemoryDAOPool::get<UserDAO, config::UserDbPath>();

    try {
        userDAO->at(1);
    } catch (...) {
        log::hxLog.warning("第一次启动! 用户名: hx, 密码: hx666 , 请尽快修改密码");
        auto nacl = utils::Uuid::makeV4();
        userDAO->add<UserDO>({
            {},
            "hx",
            {},
            nacl,
            utils::md5("hx666" + nacl),
            {},
            {},
            PermissionEnum::Administrator,
            utils::Uuid::makeV4()
        });
    }
}

#include <csignal>

container::FutureResult<bool> isStop;

int main() {
    // return 0;
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
