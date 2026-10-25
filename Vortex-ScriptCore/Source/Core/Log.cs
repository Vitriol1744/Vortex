/*
 * Created by v1tr10l7 on 13.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
namespace  Vortex
{
    public static class Log
    {
        internal enum LogLevel
        {
            Plain = 0,
            Debug = 1,
            Trace = 2,
            Info  = 4,
            Warn  = 8,
            Error = 0x10,
            Fatal = 0x20,
        }

        public static void Debug(string format, params object[] args)
        {
            unsafe { InternalCalls.Log_LogMessage(LogLevel.Debug, FormatUtils.Format(format, args)); }
        }
        public static void Trace(string format, params object[] args)
        {
            unsafe { InternalCalls.Log_LogMessage(LogLevel.Trace, FormatUtils.Format(format, args)); }
        }
        public static void Info(string format, params object[] args)
        {
            unsafe { InternalCalls.Log_LogMessage(LogLevel.Info, FormatUtils.Format(format, args)); }
        }
        public static void Warn(string format, params object[] args)
        {
            unsafe { InternalCalls.Log_LogMessage(LogLevel.Warn, FormatUtils.Format(format, args)); }
        }
        public static void Error(string format, params object[] args)
        {
            unsafe { InternalCalls.Log_LogMessage(LogLevel.Error, FormatUtils.Format(format, args)); }
        }
        public static void Fatal(string format, params object[] args)
        {
            unsafe { InternalCalls.Log_LogMessage(LogLevel.Fatal, FormatUtils.Format(format, args)); }
        }

        public static void Debug(object value)
        {
            unsafe { InternalCalls.Log_LogMessage(LogLevel.Debug, FormatUtils.Format(value)); }
        }
        public static void Trace(object value)
        {
            unsafe { InternalCalls.Log_LogMessage(LogLevel.Trace, FormatUtils.Format(value)); }
        }
        public static void Info(object value)
        {
            unsafe { InternalCalls.Log_LogMessage(LogLevel.Info, FormatUtils.Format(value)); }
        }
        public static void Warn(object value)
        {
            unsafe { InternalCalls.Log_LogMessage(LogLevel.Warn, FormatUtils.Format(value)); }
        }
        public static void Error(object value)
        {
            unsafe { InternalCalls.Log_LogMessage(LogLevel.Error, FormatUtils.Format(value)); }
        }
        public static void Fatal(object value)
        {
            unsafe { InternalCalls.Log_LogMessage(LogLevel.Fatal, FormatUtils.Format(value)); }
        }
    }
}
