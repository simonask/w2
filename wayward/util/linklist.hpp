#pragma once

#include <cassert>
#include <stddef.h>
#include <type_traits> // std::conditional_t
#include <iterator> // std::reverse_iterator

namespace wayward {
namespace util {

    struct IntrusiveListAnchor {
        ~IntrusiveListAnchor() {
            unlink();
        }

        void unlink() {
            next->prev = prev;
            prev->next = next;
            next = nullptr;
            prev = nullptr;
        }

    private:
        template <class T, IntrusiveListAnchor T::*> friend struct IntrusiveList;
        IntrusiveListAnchor* next = nullptr;
        IntrusiveListAnchor* prev = nullptr;
    };

    template <class T, class M>
    constexpr size_t offset_of_member(M T::*member) {
        return size_t(&(((T*)nullptr)->*member));
    }

    template <class T, IntrusiveListAnchor T::*Anchor>
    struct IntrusiveList {
        IntrusiveList() {
            list_.next = &list_;
            list_.prev = &list_;
        }

        // Delete copy and move constructors, because we contain pointers
        // to ourselves (sentinel).
        IntrusiveList(const IntrusiveList&) = delete;
        IntrusiveList(IntrusiveList&&) = delete;
        IntrusiveList& operator=(const IntrusiveList&) = delete;
        IntrusiveList& operator=(IntrusiveList&&) = delete;

        bool empty() const {
            return begin() == end();
        }

        void link_front(T* x) {
            auto& anchor = x->*Anchor;
            assert(anchor.next == nullptr && anchor.prev == nullptr);
            anchor.next = list_.next;
            anchor.prev = list_.next->prev;
            anchor.next->prev = &anchor;
            anchor.prev->next = &anchor;
            assert(list_.next == &anchor);
        }

        void link_back(T* x) {
            auto& anchor = x->*Anchor;
            assert(anchor.next == nullptr && anchor.prev == nullptr);
            anchor.next = list_.prev->next;
            anchor.prev = list_.prev;
            anchor.next->prev = &anchor;
            anchor.prev->next = &anchor;
            assert(list_.prev == &anchor);
        }

        void unlink(T* x) {
            auto& anchor = x->*Anchor;
            anchor.unlink();
        }

        template <bool Const> struct basic_iterator;
        using iterator = basic_iterator<false>;
        using const_iterator = basic_iterator<true>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
        iterator begin() {
            return iterator(list_.next);
        }
        iterator end() {
            return iterator(&list_);
        }
        const_iterator begin() const {
            return const_iterator(list_.next);
        }
        const_iterator end() const {
            return const_iterator(&list_);
        }
        const_iterator cbegin() const {
            return begin();
        }
        const_iterator cend() const {
            return end();
        }
        reverse_iterator rbegin() {
            return std::make_reverse_iterator(end());
        }
        reverse_iterator rend() {
            return std::make_reverse_iterator(begin());
        }
        const_reverse_iterator rbegin() const {
            return std::make_reverse_iterator(end());
        }
        const_reverse_iterator rend() const {
            return std::make_reverse_iterator(begin());
        }
        const_reverse_iterator crbegin() const {
            return rbegin();
        }
        const_reverse_iterator crend() const {
            return rend();
        }
    private:
        // Head is "next", tail is "prev".
        IntrusiveListAnchor list_;
    };

    template <class T, IntrusiveListAnchor T::*Anchor>
    template <bool Const>
    struct IntrusiveList<T, Anchor>::basic_iterator {
        using difference_type = ptrdiff_t;
        using value_type = std::conditional_t<Const, const T, T>;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::bidirectional_iterator_tag;

        template <bool OtherConst>
        bool operator==(const basic_iterator<OtherConst>& other) const {
            return it_ == other.it_;
        }

        template <bool OtherConst>
        bool operator!=(const basic_iterator<OtherConst>& other) const {
            return it_ != other.it_;
        }

        basic_iterator& operator++() {
            it_ = it_->next;
            return *this;
        }

        basic_iterator operator++(int) {
            auto copy = *this;
            it_ = it_->next;
            return copy;
        }

        basic_iterator& operator--() {
            it_ = it_->prev;
            return *this;
        }

        basic_iterator operator--(int) {
            auto copy = *this;
            it_ = it_->prev;
            return copy;
        }

        value_type& operator*() const {
            return *object_for_anchor(it_);
        }

        value_type* operator->() const {
            return object_for_anchor(it_);
        }

    private:
        friend struct IntrusiveList<T, Anchor>;
        using anchor_type = std::conditional_t<Const, const IntrusiveListAnchor, IntrusiveListAnchor>;
        explicit basic_iterator(anchor_type* anchor) : it_(anchor) {}
        anchor_type* it_;

        using data_ptr = std::conditional_t<Const, const char*, char*>;

        value_type* object_for_anchor(IntrusiveListAnchor* x) const {
            data_ptr ptr = reinterpret_cast<data_ptr>(x);
            data_ptr base = ptr - offset_of_member(Anchor);
            return reinterpret_cast<value_type*>(base);
        }
    };

} // namespace util
} // namespace wayward

