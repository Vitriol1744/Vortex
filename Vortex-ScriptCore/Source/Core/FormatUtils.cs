/*
 * Created by v1tr10l7 on 13.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
using System;

namespace Vortex
{
    internal static class FormatUtils
    {
        internal static string Format(string format, object[] args) => string.Format(format, args);
        internal static string Format(object value) => value?.ToString() ?? "null";
    }
}
