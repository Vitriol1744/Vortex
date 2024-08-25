/*
 * Created by v1tr10l7 on 18.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

// Standard Library
#include <algorithm>
#include <any>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <expected>
#include <filesystem>
#include <forward_list>
#include <fstream>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <ostream>
#include <queue>
#include <ranges>
#include <regex>
#include <set>
#include <source_location>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

// Dependencies
#pragma warning(push)
#pragma warning(disable : 26498)
#include <fmt/format.h>
#pragma warning(pop)
#include <magic_enum.hpp>

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
