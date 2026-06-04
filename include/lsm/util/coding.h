#pragma once
#include <cstdint>
#include <string>
#include <string_view>

namespace lsm
{
    inline void EncodeFixed32(char *dst, uint32_t value)
    {
        dst[0] = (value) & 0xff;
        dst[1] = (value >> 8) & 0xff;
        dst[2] = (value >> 16) & 0xff;
        dst[3] = (value >> 24) & 0xff;
    }

    inline void EncodeFixed64(char *dst, uint64_t value)
    {
        dst[0] = (value) & 0xff;
        dst[1] = (value >> 8) & 0xff;
        dst[2] = (value >> 16) & 0xff;
        dst[3] = (value >> 24) & 0xff;
        dst[4] = (value >> 32) & 0xff;
        dst[5] = (value >> 40) & 0xff;
        dst[6] = (value >> 48) & 0xff;
        dst[7] = (value >> 56) & 0xff;
    }

    inline uint32_t DecodeFixed32(const char *src)
    {
        return (static_cast<uint32_t>(static_cast<uint8_t>(src[0]))) 
            | (static_cast<uint32_t>(static_cast<uint8_t>(src[1])) << 8) 
            | (static_cast<uint32_t>(static_cast<uint8_t>(src[2])) << 16) 
            | (static_cast<uint32_t>(static_cast<uint8_t>(src[3])) << 24);
    }

    inline uint64_t DecodeFixed64(const char *src)
    {
        return (static_cast<uint64_t>(static_cast<uint8_t>(src[0]))) 
            | (static_cast<uint64_t>(static_cast<uint8_t>(src[1])) << 8) 
            | (static_cast<uint64_t>(static_cast<uint8_t>(src[2])) << 16) 
            | (static_cast<uint64_t>(static_cast<uint8_t>(src[3])) << 24) 
            | (static_cast<uint64_t>(static_cast<uint8_t>(src[4])) << 32) 
            | (static_cast<uint64_t>(static_cast<uint8_t>(src[5])) << 40) 
            | (static_cast<uint64_t>(static_cast<uint8_t>(src[6])) << 48) 
            | (static_cast<uint64_t>(static_cast<uint8_t>(src[7])) << 56);
    }

    char *EncodeVarint32(char *dst, uint32_t value);
    char *EncodeVarint64(char *dst, uint64_t value);

    void PutVarint32(std::string *dst, uint32_t value);
    void PutVarint64(std::string *dst, uint64_t value);
    void PutLengthPrefixedSlice(std::string *dst, std::string_view value);

    bool GetVarint32(std::string_view *input, uint32_t *value);
    bool GetVarint64(std::string_view *input, uint64_t *value);
    bool GetLengthPrefixedSlice(std::string_view *input, std::string_view *result);

    const char *GetVarint32Ptr(const char *p, const char *limit, uint32_t *value);
}
