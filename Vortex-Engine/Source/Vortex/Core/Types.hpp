/*
 * Created by v1tr10l7 on 17.07.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

// Prism's macros (PM_*, IgnoreUnused, Bit, PrismCheck...) and fundamental types
// live in textual headers: named modules can't export macros, and Prism.*
// modules don't re-export these types. Everything else comes from imports.
#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Types.hpp>

import Prism.Memory.Ref;
import Prism.String.String;
import Prism.String.StringView;
