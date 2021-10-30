// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#define uppriv_PP_CAT(x, ...) x##__VA_ARGS__
#define UP_PP_CAT(x, ...) uppriv_PP_CAT(x, __VA_ARGS__)

static_assert(UP_PP_CAT(tr, ue));

#define UP_PP_COMMA ,

static_assert(((void)false UP_PP_COMMA true));

#define uppriv_STRINGIFY(x) #x
#define UP_STRINGIFY(x) uppriv_STRINGIFY(x)

static_assert(UP_STRINGIFY(abcd)[2] == 'c');

#define UP_PP_IF(cond) UP_PP_CAT(uppriv_PP_IF, cond)
#define uppriv_PP_IF0(yes, no) no
#define uppriv_PP_IF1(yes, no) yes

static_assert(UP_PP_IF(1)(42, 0) == 42);
static_assert(UP_PP_IF(0)(42, 0) == 0);

#define UP_PP_IGNORE(...)
#define UP_PP_EXPAND(...) __VA_ARGS__

static_assert(UP_PP_IGNORE(2) + 2 == 2);
static_assert(((void)UP_PP_EXPAND(7, 2) + 2) == 4);

#define UP_PP_SELECT(cond) \
    UP_PP_IF(cond) \
    (UP_PP_EXPAND, UP_PP_IGNORE)

static_assert(UP_PP_SELECT(1)(2) + 2 == 4);
static_assert(UP_PP_SELECT(0)(2) + 2 == 2);

#define uppriv_PP_NUM_ARITY(_9, _8, _7, _6, _5, _4, _3, _2, _1, _0, N, ...) N
#define UP_PP_ARITY(...) UP_PP_EXPAND(uppriv_PP_NUM_ARITY(ignore, ##__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

static_assert(UP_PP_ARITY(a, b, c, d) == 4);
static_assert(UP_PP_ARITY(a) == 1);

#define uppriv_PP_JOIN0(ignore)
#define uppriv_PP_JOIN1(a) a
#define uppriv_PP_JOIN2(a, b) a##b
#define uppriv_PP_JOIN3(a, b, c) a##b##c
#define uppriv_PP_JOIN4(a, b, c, d) a##b##c##d
#define uppriv_PP_JOIN5(a, b, c, d, e) a##b##c##d##e
#define uppriv_PP_JOIN6(a, b, c, d, e, f) a##b##c##d##e##f

#define uppriv_PP_JOIN_INVOKE(n, ...) UP_PP_EXPAND(uppriv_PP_JOIN##n(__VA_ARGS__))
#define uppriv_PP_JOIN(n, ...) uppriv_PP_JOIN_INVOKE(n, __VA_ARGS__)
#define UP_PP_JOIN(...) uppriv_PP_JOIN(UP_PP_ARITY(__VA_ARGS__), __VA_ARGS__)

static_assert(UP_PP_JOIN(1, 2, 3, 4, 5, 6) == 123456);

#define uppriv_PP_MAP0(t, ignore)
#define uppriv_PP_MAP1(t, a) t(a)
#define uppriv_PP_MAP2(t, a, b) t(a), t(b)
#define uppriv_PP_MAP3(t, a, b, c) t(a), t(b), t(c)
#define uppriv_PP_MAP4(t, a, b, c, d) t(a), t(b), t(c), t(d)
#define uppriv_PP_MAP5(t, a, b, c, d, e) t(a), t(b), t(c), t(d), t(e)
#define uppriv_PP_MAP6(t, a, b, c, d, e, f) t(a), t(b), t(c), t(d), t(e), t(f)

#define uppriv_PP_MAP_INVOKE(t, n, ...) UP_PP_EXPAND(uppriv_PP_MAP##n(t, __VA_ARGS__))
#define uppriv_PP_MAP(t, n, ...) uppriv_PP_MAP_INVOKE(t, n, __VA_ARGS__)
#define UP_PP_MAP(t, ...) uppriv_PP_MAP(t, UP_PP_ARITY(__VA_ARGS__), __VA_ARGS__)

#define uppriv_PP_MAP_TEST(x) x
static_assert((UP_PP_MAP(uppriv_PP_MAP_TEST, (void)1, (void)2, 3)) == 3);
