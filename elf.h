#pragma once
#include <cstdint>
#include <cstddef>


namespace libelf {

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using usize = std::size_t;

extern "C" {
    void *memcpy(void *dest, const void *src, size_t n);
};

struct ElfHeader {
    u8 e_ident[16];
    u16 e_type;
    u16 e_machine;
    u32 e_version;
    u64 e_entry;
    u64 e_phoff;
    u64 e_shoff;
    u32 e_flags;
    u16 e_ehsize;
    u16 e_phentsize;
    u16 e_phnum;
    u16 e_shentsize;
    u16 e_shnum;
    u16 e_shstrndx;
};

struct SectionHeader {
    u32 sh_name;
    u32 sh_type;
    u64 sh_flags;
    u64 sh_addr;
    u64 sh_offset;
    u64 sh_size;
    u32 sh_link;
    u32 sh_info;
    u64 sh_addralign;
    u64 sh_entsize;
};

static_assert(sizeof(SectionHeader) == 0x40, "Size of an ELF Section Header should be 0x40 Bytes");

enum class ElfType {
    Unknown = 0x0,
    Rel,
    Exec,
    Dyn,
    Core,
    Loos = 0xFE00,
    Hios = 0xFEFF,
    Loproc = 0xFF00,
    Hiproc = 0xFFFF,
};

ElfType to_ElfType(u16 _val)
{
    return static_cast<ElfType>(_val);
}

static_assert(sizeof(ElfHeader) == 64, "ELF Header must be 64 bytes.");

class Elf {
    usize _strm_cursor{};
    usize _stream_size{};
    const u8* _stream;
    ElfHeader _header;
    SectionHeader* _string_tab = nullptr;

public:
    Elf(const u8* data, usize stream_size) : _stream{data}, _stream_size{stream_size} {
        this->_copy_n_bytes<16>(_header.e_ident);
        _header.e_type = _get_16();
        _header.e_machine = _get_16();
        _header.e_version = _get_32();
        _header.e_entry = _get_64();
        _header.e_phoff = _get_64();
        _header.e_shoff = _get_64();
        _header.e_flags = _get_32();
        _header.e_ehsize = _get_16();
        _header.e_phentsize = _get_16();
        _header.e_phnum = _get_16();
        _header.e_shentsize = _get_16();
        _header.e_shnum = _get_16();
        _header.e_shstrndx = _get_16();

        // get the address of the string table section
        auto sec_addr = (_header.e_shoff) + (sizeof(SectionHeader) * (_header.e_shstrndx));

        // TODO: dont do UB
        _string_tab = reinterpret_cast<SectionHeader*>((void*)&_stream[sec_addr]);
    }

    const ElfHeader& header() const 
    {
        return _header;
    }

    SectionHeader* sections() const
    {
        return static_cast<SectionHeader*>((void*)&_stream[_header.e_shoff]);
    }

    const char* string(u32 offset) const
    {
        // the string table address is in `sh_offset` of the section header.
        // add the offset from `sh_name` to get the starting address of the string
        return static_cast<const char*>((void*)&_stream[_string_tab->sh_offset + offset]);
    }

private:
    inline std::uint8_t _get_8()
    {
        auto _out = _stream[_strm_cursor];
        _strm_cursor += 1;
        return _out;
    }

    inline std::uint16_t _get_16()
    {
        auto _out = _stream[_strm_cursor + 1] << 8 | _stream[_strm_cursor];
        _strm_cursor += 2;
        return _out;
    }

    inline std::uint32_t _get_32()
    {
        auto _out = _stream[_strm_cursor + 3] << 24 | 
            _stream[_strm_cursor + 2]  << 16 | 
            _stream[_strm_cursor + 1] << 8 | 
            _stream[_strm_cursor];
        _strm_cursor += 4;
        return _out;
    }


    inline std::uint64_t _get_64()
    {
        auto _out = u64(_stream[_strm_cursor + 7]) << 56 |
        u64(_stream[_strm_cursor + 6]) << 48 |
        u64(_stream[_strm_cursor + 5]) << 40 |
        u64(_stream[_strm_cursor + 4]) << 32 |
        _stream[_strm_cursor + 3] << 24 | 
            _stream[_strm_cursor + 2]  << 16 | 
            _stream[_strm_cursor + 1] << 8 | 
            _stream[_strm_cursor];
        _strm_cursor += 8;
        return _out;
    }

    template <size_t N>
    inline void _copy_n_bytes(u8 buf[N])
    {
        memcpy(buf, _stream, N);
        _strm_cursor += N;
    }
};
};
