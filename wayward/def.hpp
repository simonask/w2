#pragma once


#if defined(_WIN32) || defined(__CYGWIN__)
#  if defined(_MSC_VER)
#    if defined(wayward_EXPORTS)
#      define WAYWARD_EXPORT __declspec(dllexport)
#    else
#      define WAYWARD_EXPORT __declspec(dllimport)
#    endif // wayward_EXPORTS
#  else // _MSC_VER
#    if defined(wayward_EXPORTS)
#      define WAYWARD_EXPORT __attribute__((dllexport))
#    else
#      define WAYWARD_EXPORT __attribute__((dllimport))
#    endif // wayward_EXPORTS
#  endif
#  define WAYWARD_PRIVATE
#else // _WIN32 || __CYGWIN__
#  define WAYWARD_EXPORT __attribute__((visibility("default")))
#  define WAYWARD_PRIVATE __attribute__((visibility("hidden")))
#endif
