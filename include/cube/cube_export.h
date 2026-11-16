#pragma once

/*
 * CUBE_EXPORT / CUBE_NO_EXPORT / CUBE_DEPRECATED
 *
 * To build the dynamic library:    -DCUBE_BUILDING_DLL
 * To consume the static library:   -DCUBE_USE_STATIC
 */

#if defined(CUBE_USE_STATIC) /* We are building / using static library */
#  define CUBE_EXPORT
#else
#  if defined(_WIN32)
#    if defined(CUBE_BUILDING_DLL) /* We are building this library */
#      define CUBE_EXPORT __declspec(dllexport)
#    else /* We are using this library */
#      define CUBE_EXPORT __declspec(dllimport)
#    endif
#  elif defined(__GNUC__) || defined(__clang__)
#    define CUBE_EXPORT __attribute__((visibility("default")))
#    define CUBE_NO_EXPORT __attribute__((visibility("hidden")))
#  else
#    define CUBE_EXPORT
#  endif
#endif

#ifndef CUBE_NO_EXPORT
#  define CUBE_NO_EXPORT
#endif

#if defined(__cplusplus) && __cplusplus >= 201402L
#  define CUBE_DEPRECATED(msg) [[deprecated(msg)]]
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
#  define CUBE_DEPRECATED(msg) [[deprecated(msg)]]
#elif defined(_MSC_VER)
#  define CUBE_DEPRECATED(msg) __declspec(deprecated(msg))
#elif defined(__has_attribute) && __has_attribute(deprecated)
#  define CUBE_DEPRECATED(msg) __attribute__((deprecated(msg)))
#else
#  define CUBE_DEPRECATED(msg)
#endif

#define CUBE_DEPRECATED_EXPORT(msg) CUBE_DEPRECATED(msg) CUBE_EXPORT
