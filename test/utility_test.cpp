#include <type_traits>

#include <utility/existence_verifier.hpp>
#include <utility/function_signature.hpp>
#include <utility/type_map.hpp>
#include <utility/type_set.hpp>

MAKE_EXISTENCE_VERIFIER(a);
MAKE_EXISTENCE_VERIFIER(b);
MAKE_EXISTENCE_VERIFIER(c);
MAKE_EXISTENCE_VERIFIER(d);

using namespace ctfsm;

struct A
{
    private:
        int b;

    public:
        int a;
        int c(int, float);
        int c(double);
};

static_assert(!has_b<A>);
static_assert(has_a<A>);
static_assert(!has_c<A>);
static_assert(has_c_method<A, int(int, float)>);
static_assert(!has_c_method<A, int(int)>);

using map = type_map<std::pair<int, float>, std::pair<double, char>>;

static_assert(std::is_same_v<extract_values<map>::values, std::tuple<float, char>>);
static_assert(std::is_same_v<extract_keys<map>::keys, std::tuple<int, double>>);
static_assert(std::is_same_v<find_by_key<int, map>::result, float>);
static_assert(std::is_same_v<find_by_key<double, map>::result, char>);

using set = type_set<int, int, float, double, char, int, float, int>::set;

static_assert(std::is_same_v<set, std::tuple<double, char, float, int>>);
static_assert(contains<double, set>::value);
static_assert(contains<char, set>::value);
static_assert(contains<float, set>::value);
static_assert(contains<int, set>::value);
static_assert(!contains<short, set>::value);

using merge_res = type_set_merge<std::tuple<int, float, long, short, long, long>, set>;

static_assert(contains<long, merge_res::set>::value);
static_assert(contains<short, merge_res::set>::value);
static_assert(contains<double, merge_res::set>::value);
static_assert(contains<char, merge_res::set>::value);
static_assert(contains<float, merge_res::set>::value);
static_assert(contains<int, merge_res::set>::value);

static_assert(contains<long, merge_res::delta>::value);
static_assert(contains<short, merge_res::delta>::value);
static_assert(!contains<double, merge_res::delta>::value);
static_assert(!contains<char, merge_res::delta>::value);
static_assert(!contains<float, merge_res::delta>::value);
static_assert(!contains<int, merge_res::delta>::value);

void foo(int);
int  bar(double);

using foo_signature = decltype(foo);
using bar_signature = decltype(bar);

static_assert(std::is_same_v<return_type_t<foo_signature>, void>);
static_assert(std::is_same_v<return_type_t<bar_signature>, int>);
static_assert(std::is_same_v<arg_type_t<0, foo_signature>, int>);
static_assert(std::is_same_v<arg_type_t<0, bar_signature>, double>);
static_assert(number_of_args_v<foo_signature> == 1);