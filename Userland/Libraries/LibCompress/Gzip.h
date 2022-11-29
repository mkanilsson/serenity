/*
 * Copyright (c) 2020-2022, the SerenityOS developers.
 * Copyright (c) 2021, Idan Horowitz <idan.horowitz@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibCompress/Deflate.h>
#include <LibCore/Stream.h>
#include <LibCrypto/Checksum/CRC32.h>

namespace Compress {

constexpr u8 gzip_magic_1 = 0x1f;
constexpr u8 gzip_magic_2 = 0x8b;
struct [[gnu::packed]] BlockHeader {
    u8 identification_1;
    u8 identification_2;
    u8 compression_method;
    u8 flags;
    LittleEndian<u32> modification_time;
    u8 extra_flags;
    u8 operating_system;

    bool valid_magic_number() const;
    bool supported_by_implementation() const;
};

struct Flags {
    static constexpr u8 FTEXT = 1 << 0;
    static constexpr u8 FHCRC = 1 << 1;
    static constexpr u8 FEXTRA = 1 << 2;
    static constexpr u8 FNAME = 1 << 3;
    static constexpr u8 FCOMMENT = 1 << 4;

    static constexpr u8 MAX = FTEXT | FHCRC | FEXTRA | FNAME | FCOMMENT;
};

class GzipDecompressor final : public Core::Stream::Stream {
public:
    GzipDecompressor(NonnullOwnPtr<Core::Stream::Stream>);
    ~GzipDecompressor();

    virtual ErrorOr<Bytes> read(Bytes) override;
    virtual ErrorOr<size_t> write(ReadonlyBytes) override;
    virtual bool is_eof() const override;
    virtual bool is_open() const override { return true; }
    virtual void close() override {};

    static ErrorOr<ByteBuffer> decompress_all(ReadonlyBytes);
    static Optional<DeprecatedString> describe_header(ReadonlyBytes);
    static bool is_likely_compressed(ReadonlyBytes bytes);

private:
    class Member {
    public:
        Member(BlockHeader header, Core::Stream::Stream& stream)
            : m_header(header)
            , m_adapted_ak_stream(make<Core::Stream::WrapInAKInputStream>(stream))
            , m_stream(*m_adapted_ak_stream)
        {
        }

        BlockHeader m_header;
        NonnullOwnPtr<InputStream> m_adapted_ak_stream;
        DeflateDecompressor m_stream;
        Crypto::Checksum::CRC32 m_checksum;
        size_t m_nread { 0 };
    };

    Member const& current_member() const { return m_current_member.value(); }
    Member& current_member() { return m_current_member.value(); }

    NonnullOwnPtr<Core::Stream::Stream> m_input_stream;
    u8 m_partial_header[sizeof(BlockHeader)];
    size_t m_partial_header_offset { 0 };
    Optional<Member> m_current_member;

    bool m_eof { false };
};

class GzipCompressor final : public OutputStream {
public:
    GzipCompressor(OutputStream&);
    ~GzipCompressor() = default;

    size_t write(ReadonlyBytes) override;
    bool write_or_error(ReadonlyBytes) override;

    static Optional<ByteBuffer> compress_all(ReadonlyBytes bytes);

private:
    OutputStream& m_output_stream;
};

}
