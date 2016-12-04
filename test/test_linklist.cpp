#include "wayward/util/linklist.hpp"

#include <gtest/gtest.h>

using namespace wayward::util;

struct Foo {
    int foo = 123;
    IntrusiveListAnchor anchor;
};

TEST(IntrusiveList, CannotBeMovedOrCopied) {
    using T = IntrusiveList<Foo, &Foo::anchor>;
    EXPECT_FALSE(std::is_move_constructible<T>::value);
    EXPECT_FALSE(std::is_move_assignable<T>::value);
    EXPECT_FALSE(std::is_copy_constructible<T>::value);
    EXPECT_FALSE(std::is_copy_assignable<T>::value);
}

TEST(IntrusiveList, LinkFront) {
    Foo a, b, c;
    IntrusiveList<Foo, &Foo::anchor> list;
    list.link_front(&a);
    list.link_front(&b);
    list.link_front(&c);
    EXPECT_FALSE(list.empty());
}

TEST(IntrusiveList, LinkBack) {
    Foo a, b, c;
    IntrusiveList<Foo, &Foo::anchor> list;
    list.link_back(&a);
    list.link_back(&b);
    list.link_back(&c);
    EXPECT_FALSE(list.empty());
}

TEST(IntrusiveList, ForwardIteration) {
    Foo a, b, c;
    IntrusiveList<Foo, &Foo::anchor> list;
    list.link_front(&a);
    list.link_front(&b);
    list.link_front(&c);
    auto p = list.begin();
    EXPECT_EQ(&*p, &c);
    ++p;
    EXPECT_EQ(&*p, &b);
    ++p;
    EXPECT_EQ(&*p, &a);
    ++p;
    EXPECT_EQ(p, list.end());
}

TEST(IntrusiveList, ForwardIterationLinkBack) {
    Foo a, b, c;
    IntrusiveList<Foo, &Foo::anchor> list;
    list.link_back(&a);
    list.link_back(&b);
    list.link_back(&c);
    auto p = list.begin();
    EXPECT_EQ(&*p, &a);
    ++p;
    EXPECT_EQ(&*p, &b);
    ++p;
    EXPECT_EQ(&*p, &c);
    ++p;
    EXPECT_EQ(p, list.end());
}


TEST(IntrusiveList, DecrementIterator) {
    Foo a, b, c;
    IntrusiveList<Foo, &Foo::anchor> list;
    list.link_front(&a);
    list.link_front(&b);
    list.link_front(&c);
    auto p = list.end();
    --p;
    EXPECT_EQ(&*p, &a);
    --p;
    EXPECT_EQ(&*p, &b);
    --p;
    EXPECT_EQ(&*p, &c);
    --p;
    EXPECT_EQ(p, list.end());
}

TEST(IntrusiveList, ReverseIterator) {
    Foo a, b, c;
    IntrusiveList<Foo, &Foo::anchor> list;
    list.link_front(&a);
    list.link_front(&b);
    list.link_front(&c);
    auto p = list.rbegin();
    EXPECT_EQ(&*p, &a);
    ++p;
    EXPECT_EQ(&*p, &b);
    ++p;
    EXPECT_EQ(&*p, &c);
    ++p;
    EXPECT_EQ(p, list.rend());
}

TEST(IntrusiveList, UnlinkOnDestruction) {
    IntrusiveList<Foo, &Foo::anchor> list;
    Foo a;
    list.link_front(&a);
    {
        Foo b;
        list.link_front(&b);
        auto p = list.begin();
        EXPECT_EQ(&*p, &b);
        ++p;
        EXPECT_EQ(&*p, &a);
    }
    auto p = list.begin();
    EXPECT_EQ(&*p, &a);
    ++p;
    EXPECT_EQ(p, list.end());
}

TEST(IntrusiveList, Relink) {
    IntrusiveList<Foo, &Foo::anchor> list_a;
    IntrusiveList<Foo, &Foo::anchor> list_b;
    Foo a;
    list_a.link_front(&a);
    list_a.unlink(&a);
    list_b.link_front(&a);
    EXPECT_EQ(list_a.begin(), list_a.end());
    EXPECT_EQ(&*list_b.begin(), &a);
}

TEST(IntrusiveListDeathTest, LinkTwice) {
    Foo a;
    IntrusiveList<Foo, &Foo::anchor> list;
    list.link_front(&a);
    ASSERT_DEATH(list.link_front(&a), "");
}
