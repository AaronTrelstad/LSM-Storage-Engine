#include "lsm/util/coding.h"

namespace lsm
{
    char *EncodeVarint32(char *dst, uint32_t value)
    {
        auto *ptr = reinterpret_cast<uint8_t *>(dst);
        while (value >= 0x80)
        {
            *ptr++ = (value & 0x7f) | 0x80;
            value >>= 7;
        }
        *ptr++ = value;
        return reinterpret_cast<char *>(ptr);
    }

    char *EncodeVarint64(char *dst, uint64_t value)
    {
        auto *ptr = reinterpret_cast<uint8_t *>(dst);
        while (value >= 0x80)
        {
            *ptr++ = (value & 0x7f) | 0x80;
            value >>= 7;
        }
        *ptr++ = static_cast<uint8_t>(value);
        return reinterpret_cast<char *>(ptr);
    }

    void PutVarint32(std::string *dst, uint32_t value)
    {
        char buf[5];
        char *end = EncodeVarint32(buf, value);
        dst->append(buf, end - buf);
    }

    void PutVarint64(std::string *dst, uint64_t value)
    {
        char buf[10];
        char *end = EncodeVarint64(buf, value);
        dst->append(buf, end - buf);
    }

    void PutLengthPrefixedSlice(std::string *dst, std::string_view value)
    {
        PutVarint32(dst, value.size());
        dst->append(value.data(), value.size());
    }

    const char *GetVarint32Ptr(const char *p, const char *limit, uint32_t *value)
    {
        uint32_t result = 0;
        for (uint32_t shift = 0; shift <= 28 && p < limit; shift += 7)
        {
            uint32_t byte = static_cast<uint8_t>(*p++);
            if (byte & 0x80)
            {
                result |= ((byte & 0x7f) << shift);
            }
            else
            {
                result |= (byte << shift);
                *value = result;
                return p;
            }
        }
        return nullptr;
    }

    bool GetVarint32(std::string_view *input, uint32_t *value)
    {
        const char *p = input->data();
        const char *limit = p + input->size();
        const char *q = GetVarint32Ptr(p, limit, value);
        if (q == nullptr)
            return false;
        *input = std::string_view(q, limit - q);
        return true;
    }

    bool GetVarint64(std::string_view *input, uint64_t *value)
    {
        uint64_t result = 0;
        const char *p = input->data();
        const char *limit = p + input->size();

        for (uint32_t shift = 0; shift <= 63 && p < limit; shift += 7)
        {
            uint64_t byte = static_cast<uint8_t>(*p++);
            if (byte & 0x80)
            {
                result |= ((byte & 0x7f) << shift);
            }
            else
            {
                result |= (byte << shift);
                *value = result;
                *input = std::string_view(p, limit - p);
                return true;
            }
        }
        return false;
    }

    bool GetLengthPrefixedSlice(std::string_view *input, std::string_view *result)
    {
        uint32_t len;
        if (!GetVarint32(input, &len))
            return false;
        if (input->size() < len)
            return false;
        *result = std::string_view(input->data(), len);
        *input = std::string_view(input->data() + len, input->size() - len);
        return true;
    }
}
