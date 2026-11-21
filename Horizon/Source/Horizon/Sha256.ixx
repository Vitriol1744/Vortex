/*
 * Created by v1tr10l7 on 18.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
module;

#include <Prism/Core/Types.hpp>

#include <fstream>

export module Horizon.Sha256;

import Horizon.Platform;

export namespace Horizon
{
    /// SHA-256 (FIPS 180-4), for checking downloads against the manifest.
    class Sha256
    {
      public:
        Sha256() { Reset(); }

        void Reset()
        {
            constexpr u32 kInitial[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                                         0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
            for (usize i = 0; i < 8; ++i) m_State[i] = kInitial[i];
            m_Length    = 0;
            m_BlockSize = 0;
        }

        void Update(const void* data, usize size)
        {
            auto bytes = static_cast<const u8*>(data);
            m_Length += size;
            while (size > 0)
            {
                usize take = size < kBlock - m_BlockSize ? size : kBlock - m_BlockSize;
                for (usize i = 0; i < take; ++i)
                    m_Block[m_BlockSize + i] = bytes[i];
                m_BlockSize += take;
                bytes += take;
                size -= take;
                if (m_BlockSize == kBlock)
                {
                    Transform(m_Block);
                    m_BlockSize = 0;
                }
            }
        }
        void   Update(StringView data) { Update(data.Raw(), data.Size()); }

        /// Lowercase hex digest; the hasher must be Reset() before reuse.
        String Finish()
        {
            u64 bits    = m_Length * 8;
            u8  pad[72] = {0x80};
            usize   padSize = (m_BlockSize < 56 ? 56 : 120) - m_BlockSize;
            for (int i = 0; i < 8; ++i)
                pad[padSize + i] = static_cast<u8>(bits >> (56 - 8 * i));
            Update(pad, padSize + 8);

            constexpr char kHex[] = "0123456789abcdef";
            String         digest;
            for (u32 word : m_State)
                for (int shift = 28; shift >= 0; shift -= 4)
                    digest += kHex[(word >> shift) & 0xf];
            return digest;
        }

      private:
        static constexpr usize kBlock = 64;
        u32                    m_State[8]{};
        u8                     m_Block[kBlock]{};
        usize                  m_BlockSize = 0;
        u64                m_Length    = 0;

        static constexpr u32 Rotr(u32 x, int n)
        {
            return (x >> n) | (x << (32 - n));
        }

        void Transform(const u8* block)
        {
            static constexpr u32 k[64] = {
                0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b,
                0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01,
                0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7,
                0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
                0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152,
                0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
                0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc,
                0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
                0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819,
                0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116, 0x1e376c08,
                0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f,
                0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
                0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

            u32 w[64];
            for (int i = 0; i < 16; ++i)
                w[i] = (u32(block[i * 4]) << 24)
                     | (u32(block[i * 4 + 1]) << 16)
                     | (u32(block[i * 4 + 2]) << 8)
                     | u32(block[i * 4 + 3]);
            for (int i = 16; i < 64; ++i)
            {
                u32 s0
                    = Rotr(w[i - 15], 7) ^ Rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
                u32 s1
                    = Rotr(w[i - 2], 17) ^ Rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
                w[i] = w[i - 16] + s0 + w[i - 7] + s1;
            }

            auto [a, b, c, d, e, f, g, h] = m_State;
            for (int i = 0; i < 64; ++i)
            {
                u32 s1  = Rotr(e, 6) ^ Rotr(e, 11) ^ Rotr(e, 25);
                u32 ch  = (e & f) ^ (~e & g);
                u32 t1  = h + s1 + ch + k[i] + w[i];
                u32 s0  = Rotr(a, 2) ^ Rotr(a, 13) ^ Rotr(a, 22);
                u32 maj = (a & b) ^ (a & c) ^ (b & c);
                u32 t2  = s0 + maj;
                h                 = g;
                g                 = f;
                f                 = e;
                e                 = d + t1;
                d                 = c;
                c                 = b;
                b                 = a;
                a                 = t1 + t2;
            }
            m_State[0] += a;
            m_State[1] += b;
            m_State[2] += c;
            m_State[3] += d;
            m_State[4] += e;
            m_State[5] += f;
            m_State[6] += g;
            m_State[7] += h;
        }
    };

    /// Hex SHA-256 of a file's contents; nullopt if it can't be read.
    Optional<String> FileSha256(PathView path)
    {
        std::ifstream file(ToFs(path), std::ios::binary);
        if (!file) return NullOpt;

        Sha256 hasher;
        char   buffer[64 * 1024];
        while (file)
        {
            file.read(buffer, sizeof(buffer));
            hasher.Update(buffer, static_cast<usize>(file.gcount()));
        }
        if (file.bad()) return NullOpt;
        return hasher.Finish();
    }
} // namespace Horizon
