#include "wayward/util/string_view.hpp"

#include <string>

namespace wayward {
namespace util {

StringView::operator std::string() const {
    return std::string(s_, size_);
}

} // namespace util
} // namespace wayward
