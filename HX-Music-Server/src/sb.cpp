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

auto sb___ = [] {
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
    return 0;
}();