#ifndef CONFIG_H
#define CONFIG_H

#define BLACKHOLE_VERSION_MAJOR 0
#define BLACKHOLE_VERSION_MINOR 4
#define BLACKHOLE_VERSION_PATCH 0
#define BLACKHOLE_HEADER_ONLY
#define BLACKHOLE_DEBUG

#if defined(BLACKHOLE_HEADER_ONLY)
    #define BLACKHOLE_API inline
#endif

// If BLACKHOLE_API isn't defined yet define it now.
#if !defined(BLACKHOLE_API)
    #define BLACKHOLE_API
#endif

// Add process id attribute to every logging event. Attribute has integer type.
#define BLACKHOLE_HAS_ATTRIBUTE_PID

// Add thread id attribute to every logging event. Attribute is represented as hex-formatted string.
#define BLACKHOLE_HAS_ATTRIBUTE_TID

// Add lightweight process id attribute to every logging event. Attribute has integer type.
#undef  BLACKHOLE_HAS_ATTRIBUTE_LWP

#define BLACKHOLE_INTERNAL_SET_RESERVED_SIZE 6
#define BLACKHOLE_EXTERNAL_SET_RESERVED_SIZE 16

// Enable format expression syntax checking in type-safe manner.
// 0 - disable.
// 1 - using printf GNU extension.
#ifndef BLACKHOLE_CHECK_FORMAT_SYNTAX
    #define BLACKHOLE_CHECK_FORMAT_SYNTAX 1
#endif

#endif // CONFIG_H
