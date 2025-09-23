#include <db/SQLiteDB.hpp>

using namespace HX;

struct Wdf {
    template <bool... Bs, typename... Ts>
    std::size_t func(db::FieldPair<Ts>...) {
        using Ptr = std::size_t (Wdf::*)(db::FieldPair<Ts>...);
        constexpr Ptr ptr = &Wdf::func<Bs...>;
        return meta::TypeId::make<ptr>();
    }
};

// test
auto sb___ = [] {
    return 0;
    struct A {
        int a, b, c;
    };
    struct B {
        int a;
    };
    using sb = meta::GetMemberPtrsClassType<
        decltype(&A::a),
        decltype(&A::b),
        decltype(&A::c)
    >;
    log::hxLog.info(
        Wdf{}.func<false>(
            db::FieldPair{&A::a, 1}
        ),
        Wdf{}.func<true>(
            db::FieldPair{&A::a, 1}
        ),
        Wdf{}.func<false>(
            db::FieldPair{&A::a, 1},
            db::FieldPair{&A::b, 1}
        )
    );

    db::SQLiteDB db{"del.db"};
    struct MyTable {
        db::PrimaryKey<uint64_t> id;
        std::string name;
    };
    db.createDatabase<MyTable>();
    db.insertBy(db::FieldPair{
        &MyTable::name, "张三"
    });
    db.insertBy(db::FieldPair{
        &MyTable::name, "张三"
    });
    log::hxLog.info(db.queryAll<MyTable>());
    db.updateBy<"where name = ?">(db::FieldPair{
        &MyTable::name, "李四"
    }).bind<true>(std::string{"张三"}).execOnThrow();
    log::hxLog.info(db.queryAll<MyTable>());
    db.deleteBy<MyTable>("where id = 1").execOnThrow();
    log::hxLog.info(db.queryAll<MyTable>());
    return 0;
}();