#pragma once

#include <stddef.h> // size_t
#include <cstring> // strlen, etc.
#include <cassert>

#include <stdexcept> // std::out_of_range
#include <iosfwd> // std::basic_ostream
#include <iterator> // std::make_reverse_iterator

namespace wayward {
namespace util {

// TODO: Eliminate this in C++17
struct StringView {
    static constexpr size_t npos = size_t(-1);

    constexpr StringView() : s_(nullptr), size_(0) {}
    constexpr StringView(const StringView&) = default;
    constexpr StringView(const char* s, size_t sz) : s_(s), size_(sz) {}
    StringView(const char* s) : StringView(s, std::strlen(s)) {}
    template <size_t N>
    constexpr StringView(const char(&s)[N]);

    // XXX: Divergence from C++17, which has a constructor on std::string.
    StringView(const std::string&);
    // XXX: Divergence from C++17, which has a conversion operator on std::string.
    operator std::string() const;

    constexpr StringView& operator=(const StringView&) = default;

    using const_iterator = const char*;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    constexpr const_iterator begin() const { return s_; }
    constexpr const_iterator cbegin() const { return s_; }
    const_reverse_iterator rbegin() const { return std::make_reverse_iterator(end()); }
    const_reverse_iterator crbegin() const { return rbegin(); }
    constexpr const_iterator end() const { return s_ + size_; }
    constexpr const_iterator cend() const { return end(); }
    const_reverse_iterator rend() const { return std::make_reverse_iterator(begin()); }
    const_reverse_iterator crend() const { return rend(); }

    char operator[](size_t idx) const { return at(idx); }
    char at(size_t idx) const { if (idx > size_) throw std::out_of_range("out of range"); return s_[idx]; }
    char front() const { return at(0); }
    char back() const { return at(size_-1); }
    const char* data() const { return s_; }

    constexpr size_t size() const { return size_; }
    constexpr size_t length() const { return size_; }
    constexpr size_t max_size() const { return size_t(-1); }
    constexpr bool empty() const { return size_ != 0; }

    void remove_prefix(size_t n) { assert(n < size_); s_ += n; size_ -= n; }
    void remove_suffix(size_t n) { assert(n < size_); size_ -= n; }

    void swap(StringView& other) { std::swap(s_, other.s_); std::swap(size_, other.size_); }

    size_t copy(char* dest, size_t count, size_t pos = 0) const;
    constexpr StringView substr(size_t pos = 0, size_t count = npos) const;

    int compare(StringView) const;
    int compare(size_t pos1, size_t count1, StringView) const;
    int compare(size_t pos1, size_t count1, StringView v, size_t pos2, size_t count2) const;
    int compare(const char* s) const;
    int compare(size_t pos1, size_t count1, const char* s) const;
    int compare(size_t pos1, size_t count1, const char* s, size_t count2) const;

    size_t find(StringView, size_t pos = 0) const;
    size_t find(char c, size_t pos = 0) const;
    size_t find(const char* s, size_t pos, size_t count) const;
    size_t find(const char* s, size_t pos = 0) const;

    size_t rfind(StringView, size_t pos = 0) const;
    size_t rfind(char c, size_t pos = 0) const;
    size_t rfind(const char* s, size_t pos, size_t count) const;
    size_t rfind(const char* s, size_t pos = 0) const;

    size_t find_first_of(StringView, size_t pos = 0) const;
    size_t find_first_of(char c, size_t pos = 0) const;
    size_t find_first_of(const char* s, size_t pos, size_t count) const;
    size_t find_first_of(const char* s, size_t pos = 0) const;

    size_t find_last_of(StringView, size_t pos = 0) const;
    size_t find_last_of(char c, size_t pos = 0) const;
    size_t find_last_of(const char* s, size_t pos, size_t count) const;
    size_t find_last_of(const char* s, size_t pos = 0) const;

    size_t find_first_not_of(StringView, size_t pos = 0) const;
    size_t find_first_not_of(char c, size_t pos = 0) const;
    size_t find_first_not_of(const char* s, size_t pos, size_t count) const;
    size_t find_first_not_of(const char* s, size_t pos = 0) const;

    size_t find_last_not_of(StringView, size_t pos = 0) const;
    size_t find_last_not_of(char c, size_t pos = 0) const;
    size_t find_last_not_of(const char* s, size_t pos, size_t count) const;
    size_t find_last_not_of(const char* s, size_t pos = 0) const;
private:
    const char* s_;
    size_t size_;
};

inline bool operator==(StringView a, StringView b) { return a.compare(b) == 0; }
inline bool operator!=(StringView a, StringView b) { return a.compare(b) != 0; }
inline bool operator< (StringView a, StringView b) { return a.compare(b) <  0; }
inline bool operator<=(StringView a, StringView b) { return a.compare(b) <= 0; }
inline bool operator> (StringView a, StringView b) { return a.compare(b) >  0; }
inline bool operator>=(StringView a, StringView b) { return a.compare(b) >= 0; }
std::ostream& operator<<(std::ostream& os, StringView);

namespace literals {
    StringView operator""_sv(const char* str, size_t len) noexcept;
}

/// Implementation:

constexpr StringView StringView::substr(size_t pos, size_t count) const {
    if (pos > size())
        throw std::out_of_range("pos > size()");
    if (pos + count > size())
        throw std::out_of_range("pos + count > size()");
    return StringView{s_ + pos, count};
}

inline int StringView::compare(size_t pos1, size_t count1, StringView b) const {
    return substr(pos1, count1).compare(b);
}
inline int StringView::compare(size_t pos1, size_t count1, StringView b, size_t pos2, size_t count2) const {
    return substr(pos1, count1).compare(b.substr(pos2, count2));
}
inline int StringView::compare(const char* s) const {
    return compare(StringView{s});
}
inline int StringView::compare(size_t pos1, size_t count1, const char* s) const {
    return compare(pos1, count1, StringView{s});
}
inline int StringView::compare(size_t pos1, size_t count1, const char* s, size_t count2) const {
    return compare(pos1, count1, StringView{s}, 0, count2);
}


} // namespace util
} // namespace wayward

namespace std {
    template <>
    struct hash<wayward::util::StringView> {
        size_t operator()(const wayward::util::StringView& s) const;
    };
}
