#pragma once

#include <redasm/disassembler/listing/backend/blocks/blockitem.h>
#include <redasm/disassembler/listing/backend/blocks/blockcontainer.h>
#include <deque>

namespace REDasm {

class BlockContainerImpl
{
    public:
        typedef std::deque<BlockItem> Container;

    public:
        BlockContainerImpl() = default;
        void unexplored(const BlockItem* blockitem);
        BlockItem* unexplored(address_t start);
        BlockItem* unexplored(address_t start, address_t end);
        BlockItem* data(address_t start, address_t end, BlockItemFlags flags);
        BlockItem* code(address_t start, address_t end, BlockItemFlags flags);
        BlockItem* unexploredSize(address_t start, size_t size);
        BlockItem* dataSize(address_t start, size_t size, BlockItemFlags flags);
        BlockItem* codeSize(address_t start, size_t size, BlockItemFlags flags);

    public:
        bool empty() const;
        size_t size() const;
        const BlockItem* at(size_t idx) const;
        const BlockItem* find(address_t address) const;
        BlockItem* at(size_t idx);

    private:
        BlockItem *mark(address_t start, address_t end, BlockItemType type, BlockItemFlags flags);
        BlockItem *markSize(address_t start, size_t size, BlockItemType type, BlockItemFlags flags);
        void remove(address_t start, address_t end);
        BlockItem *insert(address_t start, address_t end, BlockItemType type, BlockItemFlags flags);
        Container::const_iterator findOverlap(address_t address) const;
        Container::iterator insertionPoint(address_t address);

    private:
        Container m_blocks;
};

} // namespace REDasm