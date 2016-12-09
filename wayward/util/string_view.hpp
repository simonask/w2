#pragma once

#include <stddef.h> // size_t
#include <cstring> // strlen, etc.

#include <stdexcept> // std::out_of_range
#include <iosfwd> // std::basic_ostream

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
    constexpr StringView(const std::string&);
    // XXX: Divergence from C++17, which has a conversion operator on std::string.
    operator std::string() const;

    constexpr StringView& operator=(const StringView&) = default;

    using const_iterator = const char*;
    constexpr const_iterator begin() const;
    constexpr const_iterator cbegin() const;
    constexpr const_iterator rbegin() const;
    constexpr const_iterator crbegin() const;
    constexpr const_iterator end() const;
    constexpr const_iterator cend() const;
    constexpr const_iterator rend() const;
    constexpr const_iterator crend() const;

    char operator[](size_t) const;
    char at(size_t) const;
    char front() const;
    char back() const;
    const char* data() const;

    constexpr size_t size() const;
    constexpr size_t length() const;
    constexpr size_t max_size() const;
    constexpr bool empty() const;

    void remove_prefix(size_t);
    void remove_suffix(size_t);

    void swap(StringView&);

    size_t copy(char* dest, size_t count, size_t pos = 0) const;
    constexpr StringView substr(size_t pos = 0, size_t count = npos) const;

    constexpr int compare(StringView) const;
    constexpr int compare(size_t pos1, size_t count1, StringView) const;
    constexpr int compare(size_t pos1, size_t count1, StringView v, size_t pos2, size_t count2) const;
    constexpr int compare(const char* s) const;
    constexpr int compare(size_t pos1, size_t count1, const char* s) const;
    constexpr int compare(size_t pos1, size_t count1, const char* s, size_t count2) const;

    constexpr size_t find(StringView, size_t pos = 0) const;
    constexpr size_t find(char c, size_t pos = 0) const;
    constexpr size_t find(const char* s, size_t pos, size_t count) const;
    constexpr size_t find(const char* s, size_t pos = 0) const;

    constexpr size_t rfind(StringView, size_t pos = 0) const;
    constexpr size_t rfind(char c, size_t pos = 0) const;
    constexpr size_t rfind(const char* s, size_t pos, size_t count) const;
    constexpr size_t rfind(const char* s, size_t pos = 0) const;

    constexpr size_t find_first_of(StringView, size_t pos = 0) const;
    constexpr size_t find_first_of(char c, size_t pos = 0) const;
    constexpr size_t find_first_of(const char* s, size_t pos, size_t count) const;
    constexpr size_t find_first_of(const char* s, size_t pos = 0) const;

    constexpr size_t find_last_of(StringView, size_t pos = 0) const;
    constexpr size_t find_last_of(char c, size_t pos = 0) const;
    constexpr size_t find_last_of(const char* s, size_t pos, size_t count) const;
    constexpr size_t find_last_of(const char* s, size_t pos = 0) const;

    constexpr size_t find_first_not_of(StringView, size_t pos = 0) const;
    constexpr size_t find_first_not_of(char c, size_t pos = 0) const;
    constexpr size_t find_first_not_of(const char* s, size_t pos, size_t count) const;
    constexpr size_t find_first_not_of(const char* s, size_t pos = 0) const;

    constexpr size_t find_last_not_of(StringView, size_t pos = 0) const;
    constexpr size_t find_last_not_of(char c, size_t pos = 0) const;
    constexpr size_t find_last_not_of(const char* s, size_t pos, size_t count) const;
    constexpr size_t find_last_not_of(const char* s, size_t pos = 0) const;
private:
    const char* s_;
    size_t size_;
};

constexpr bool operator==(StringView, StringView);
constexpr bool operator!=(StringView, StringView);
constexpr bool operator<(StringView, StringView);
constexpr bool operator<=(StringView, StringView);
constexpr bool operator>(StringView, StringView);
constexpr bool operator>=(StringView, StringView);

std::ostream& operator<<(std::ostream& os, StringView);

namespace literals {
    StringView operator""_sv(const char* str, size_t len) noexcept;
}

} // namespace util
} // namespace wayward

namespace std {
    template <>
    struct hash<wayward::util::StringView> {
        size_t operator()(const wayward::util::StringView& s) const;
    };
}
