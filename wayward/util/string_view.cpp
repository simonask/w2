#include "wayward/util/string_view.hpp"

#include <string>
#include <algorithm> // std::lexicographical_compare, std::find
#include <ostream>

namespace wayward {
namespace util {

StringView::StringView(const std::string& s) : s_(s.data()), size_(s.size()) {}

StringView::operator std::string() const {
    return std::string(s_, size_);
}

int StringView::compare(StringView b) const {
    return std::lexicographical_compare(begin(), end(), b.begin(), b.end());
}

size_t StringView::find(char c, size_t pos) const {
    if (pos > size())
        throw std::out_of_range("pos > size()");
    auto it = std::find(begin() + pos, end(), c);
    if (it == end())
        return npos;
    return it - begin();
}

std::ostream& operator<<(std::ostream& os, StringView str) {
    os.write(str.data(), str.size());
    return os;
}

} // namespace util
} // namespace wayward
