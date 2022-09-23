//
// Created by Vitriol1744 on 17.09.2022.
//
#ifndef VORTEX_VTPCH_HPP
#define VORTEX_VTPCH_HPP

// Precompiled Header
// Standard Library Headers
#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <source_location>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <cmath>
#include <cstdint>

// GLM Headers
#define GLM_FORCE_INLINE
#define GLM_FORCE_INTRINSICS
#ifdef VT_32BIT_PLATFORM
    #define GLM_FORCE_SINGLE_ONLY
#endif
#include <glm/mat4x4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat2x2.hpp>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp> // perspective, translate, rotate
#include <glm/gtc/type_ptr.hpp>
#include <glm/exponential.hpp>        // all the GLSL exponential functions: pow, log, exp2, sqrt, etc.
#include <glm/trigonometric.hpp>      // all the GLSL trigonometric functions: radians, cos, asin, etc.

// FMT Headers
#include <fmt/format.h>

#endif //VORTEX_VTPCH_HPP
