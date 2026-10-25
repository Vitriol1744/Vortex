
#ifndef VT_RENDERER_RHI_VULKAN_API_H
#define VT_RENDERER_RHI_VULKAN_API_H

#ifdef VORTEX_VULKAN_STATIC_DEFINE
#  define VT_RENDERER_RHI_VULKAN_API
#  define VORTEX_VULKAN_NO_EXPORT
#else
#  ifndef VT_RENDERER_RHI_VULKAN_API
#    ifdef Vortex_Vulkan_EXPORTS
        /* We are building this library */
#      define VT_RENDERER_RHI_VULKAN_API __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define VT_RENDERER_RHI_VULKAN_API __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef VORTEX_VULKAN_NO_EXPORT
#    define VORTEX_VULKAN_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef VORTEX_VULKAN_DEPRECATED
#  define VORTEX_VULKAN_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef VORTEX_VULKAN_DEPRECATED_EXPORT
#  define VORTEX_VULKAN_DEPRECATED_EXPORT VT_RENDERER_RHI_VULKAN_API VORTEX_VULKAN_DEPRECATED
#endif

#ifndef VORTEX_VULKAN_DEPRECATED_NO_EXPORT
#  define VORTEX_VULKAN_DEPRECATED_NO_EXPORT VORTEX_VULKAN_NO_EXPORT VORTEX_VULKAN_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef VORTEX_VULKAN_NO_DEPRECATED
#    define VORTEX_VULKAN_NO_DEPRECATED
#  endif
#endif

#endif /* VT_RENDERER_RHI_VULKAN_API_H */
