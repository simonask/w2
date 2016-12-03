#pragma once

#include <cassert>
#include <stddef.h>
#include <type_traits>

namespace wayward {
namespace util {

    template <class T>
    struct IntrusiveListAnchor {
        ~IntrusiveListAnchor() {
            unlink();
        }

        void unlink() {
            next->update = update;
            *update = next;
            next = nullptr;
            update = nullptr;
        }

        IntrusiveListAnchor<T>* next = nullptr;
        IntrusiveListAnchor<T>** update = nullptr;
    };

    template <class T, class M>
    constexpr size_t offset_of_member(M T::*member) {
        T* base = nullptr;
        return size_t(&(base->*member));
    }

    template <class T, IntrusiveListAnchor<T> T::*Anchor>
    struct IntrusiveList {
        IntrusiveList() {
            list_.next = &list_;
            list_.update = &list_.next;
        }

        // Delete copy and move constructors, because we contain pointers
        // to ourselves.
        IntrusiveList(const IntrusiveList&) = delete;
        IntrusiveList(IntrusiveList&&) = delete;
        IntrusiveList& operator=(const IntrusiveList&) = delete;
        IntrusiveList& operator=(IntrusiveList&&) = delete;

        bool empty() const {
            return begin() == end();
        }

        void link_front(T* x) {
            auto& anchor = x->*Anchor;
            assert(anchor.next == nullptr && anchor.update == nullptr);
            anchor.next = list_.next;
            list_.next->update = &anchor.next;
            list_.next = &(x->*Anchor);
            anchor.update = &list_.next;
        }

        void unlink(T* x) {
            auto& anchor = x->*Anchor;
            anchor.unlink();
        }

        template <bool Const> struct basic_iterator;
        using iterator = basic_iterator<false>;
        using const_iterator = basic_iterator<true>;
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
    private:

        IntrusiveListAnchor<T> list_;
    };

    template <class T, IntrusiveListAnchor<T> T::*Anchor>
    template <bool Const>
    struct IntrusiveList<T, Anchor>::basic_iterator {
        using value_type = std::conditional_t<Const, const T, T>;

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
            ++copy;
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
        using anchor_type = std::conditional_t<Const, const IntrusiveListAnchor<T>, IntrusiveListAnchor<T>>;
        explicit basic_iterator(anchor_type* anchor) : it_(anchor) {}
        anchor_type* it_;

        using data_ptr = std::conditional_t<Const, const char*, char*>;

        value_type* object_for_anchor(IntrusiveListAnchor<T>* x) const {
            data_ptr ptr = reinterpret_cast<data_ptr>(x);
            data_ptr base = ptr - offset_of_member(Anchor);
            return reinterpret_cast<value_type*>(base);
        }
    };

} // namespace util
} // namespace wayward

