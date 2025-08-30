#include <HXLibs/log/Log.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/embed.h> // pybind11::scoped_interpreter

namespace py = pybind11;

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

int _main() {
    py::scoped_interpreter guard{};

    // 添加 Python 脚本目录到 sys.path
    py::module sys = py::module::import("sys");
    sys.attr("path").cast<py::list>().append("./pyTool");

    // 导入模块
    py::module myscript = py::module::import("main");

    myscript.attr("lddcAssToKaraokeAss")("./pyTool/test.ass", "./cpp.ass");

    return 0;
}

#include <db/SQLiteDB.hpp>
#include <db/SQLiteMeta.hpp>
#include <HXLibs/reflection/json/JsonRead.hpp>
#include <HXLibs/reflection/json/JsonWrite.hpp>

using namespace HX;

struct Man {
    db::PrimaryKey<int> id;
    std::string name;
    double okane;
    std::vector<std::string> arr;
};

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

} // namespace HX::db

#include <dao/ThreadSafeInMemoryDAO.hpp>

int _x_main() {
    if (0) {
        auto db = db::open("./test.db");
        db.createDatabase<Man>();
        auto id = db.insert<Man>({
            {}, "战士", 0.721, {"1", "a", "#"}
        });
        auto res = db.queryAll<Man>();
        log::hxLog.info("res:", res);
    
        db.deleteBy<Man>("where id = ?").bind(2).exec();
        db.updateBy<Man>({
            {}, "xxbb", 6.66, {"在", "あの場所"}
        }, "").exec();
    
        res = db.queryAll<Man>();
        log::hxLog.warning("res:", res);
    }

    // ===
    dao::ThreadSafeInMemoryDAO<Man> manDao{db::open("./test.db")};
    auto const& man = manDao.add<Man>({
        {}, "张三", 123.456, {"这", "实际上", "是", "std::vector"}
    });

    log::hxLog.debug(man);

    manDao.del(man.id);

    return 0;
}

#include <api/MusicApi.hpp>
#include <api/PlaylistApi.hpp>

#include <filesystem>

void ininDir() {
    // 创建文件夹
    std::filesystem::create_directories("file/music");
    std::filesystem::create_directories("file/db");

    // 测试数据
    [&] {
        auto playlistDAO 
            = dao::MemoryDAOPool::get<PlaylistDAO, "./file/db/playlist.db">();
        using PathStr = meta::ToCharPack<"./file/db/playlist.db">;
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
    }();
}

int main() {
    ininDir();
    net::HttpServer server{"0.0.0.0", "28205"};
    api::addApi<MusicApi>(server);
    api::addApi<PlaylistApi>(server);
    server.syncRun();
    return 0;
}
