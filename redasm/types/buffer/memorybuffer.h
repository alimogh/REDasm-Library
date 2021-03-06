#pragma once

#include <string>
#include "abstractbuffer.h"

namespace REDasm {
namespace Buffer {

class MemoryBuffer: public AbstractBuffer
{
    public:
        MemoryBuffer();
        MemoryBuffer(u64 size);
        MemoryBuffer(u64 size, u8 val);
        MemoryBuffer(const MemoryBuffer&) = delete;
        MemoryBuffer(MemoryBuffer&&mb) noexcept;
        ~MemoryBuffer();
        u8* data() const override;
        u64 size() const override;
        void resize(u64 size) override;
        void swap(MemoryBuffer& mb);

    public:
        static MemoryBuffer* fromFile(const std::string& file);

    private:
        u8* m_data;
        u64 m_size;
};

} // namespace Buffer

using MemoryBuffer = Buffer::MemoryBuffer;

} // namespace REDasm
