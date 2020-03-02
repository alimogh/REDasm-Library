#include "listingdocument_impl.h"
#include <redasm/context.h>
#include <cassert>

namespace REDasm {

ListingDocumentTypeImpl::ListingDocumentTypeImpl(ListingDocumentType *q): m_pimpl_q(q)
{
    EventManager::subscribe_m(StandardEvents::Document_BlockInserted, this, &ListingDocumentTypeImpl::onBlockInserted);
    EventManager::subscribe_m(StandardEvents::Document_BlockErased, this, &ListingDocumentTypeImpl::onBlockErased);
}

ListingDocumentTypeImpl::~ListingDocumentTypeImpl() { EventManager::ungroup(this); }
const BlockContainer* ListingDocumentTypeImpl::blocks() const { return &m_blocks; }
const ListingItems* ListingDocumentTypeImpl::items() const { return &m_items; }
const ListingSegments *ListingDocumentTypeImpl::segments() const { return &m_segments; }
const ListingFunctions *ListingDocumentTypeImpl::functions() const { return &m_functions; }
const SymbolTable* ListingDocumentTypeImpl::symbols() const { return &m_symbols; }
const InstructionCache* ListingDocumentTypeImpl::instructions() const { return &m_instructions; }
const ListingCursor& ListingDocumentTypeImpl::cursor() const { return m_cursor; }
ListingCursor& ListingDocumentTypeImpl::cursor() { return m_cursor; }

ListingItem ListingDocumentTypeImpl::functionStart(address_t address) const
{
    const BlockItem* block = m_blocks.find(address);
    if(!block) return ListingItem();

    auto location = m_functions.functionFromAddress(address);
    if(!location.valid) return ListingItem();

    PIMPL_Q(const ListingDocumentType);
    return q->itemFunction(location);
}

void ListingDocumentTypeImpl::symbol(address_t address, type_t type, flag_t flags, tag_t tag) { this->symbol(address, SymbolTable::name(address, type, flags), type, flags, tag); }

void ListingDocumentTypeImpl::symbol(address_t address, const String& name, type_t type, flag_t flags, tag_t tag)
{
    if(!this->canSymbolizeAddress(address, type, flags)) return;

    const Symbol* symbol = m_symbols.get(address);

    if(symbol)
    {
        if(symbol->isFunction())
        {
            if(type == Symbol::T_Function) // Overwrite symbol only
            {
                this->createSymbol(address, name, type, flags, tag);
                this->notify(m_items.functionIndex(address), ListingDocumentChangedEventArgs::Changed);
                return;
            }

            this->remove(address, ListingItem::FunctionItem);
        }
        else
            this->remove(address, ListingItem::SymbolItem);
    }

    this->createSymbol(address, name, type, flags, tag);

    this->insert(address, (type == Symbol::T_Function) ? ListingItem::FunctionItem :
                                                           ListingItem::SymbolItem);
}

const ListingItem& ListingDocumentTypeImpl::insert(address_t address, type_t type, size_t index)
{
    switch(type)
    {
        case ListingItem::FunctionItem:  m_functions.insert(address); this->insert(address, ListingItem::EmptyItem); break;
        case ListingItem::TypeItem: this->insert(address, ListingItem::EmptyItem); break;
        case ListingItem::SeparatorItem: m_separators.insert(address); break;
        default: break;
    }

    size_t idx = m_items.insert(address, type, index);
    this->notify(idx, ListingDocumentChangedEventArgs::Inserted);
    return m_items.at(idx);
}

void ListingDocumentTypeImpl::notify(size_t idx, size_t action)
{
    if(idx >= m_items.size()) return;
    EventManager::trigger(StandardEvents::Document_Changed, ListingDocumentChangedEventArgs(m_items.at(idx), idx, action));
}

void ListingDocumentTypeImpl::block(address_t address, size_t size, type_t type, flag_t flags, tag_t tag) { this->block(address, size, String(), type, flags, tag); }

void ListingDocumentTypeImpl::block(address_t address, size_t size, const String& name, type_t type, flag_t flags, tag_t tag)
{
    if(!this->canSymbolizeAddress(address, type, flags)) return;

    m_blocks.dataSize(address, size);
    this->symbol(address, name, type, flags, tag);
}

void ListingDocumentTypeImpl::block(const CachedInstruction& instruction) { m_blocks.codeSize(instruction->address, instruction->size); }
void ListingDocumentTypeImpl::unexplored(address_t address, size_t size) { m_blocks.unexploredSize(address, size); }

bool ListingDocumentTypeImpl::rename(address_t address, const String& name)
{
    if(!m_symbols.rename(address, name)) return false;

    this->notify(m_items.symbolIndex(address));
    return true;
}

const Symbol* ListingDocumentTypeImpl::symbol(address_t address) const
{
    const Symbol* symbol =  m_symbols.get(address);
    if(symbol) return symbol;

    const BlockItem* item = m_blocks.find(address);
    if(item) return m_symbols.get(item->start);
    return nullptr;
}

const Symbol* ListingDocumentTypeImpl::symbol(const String& name) const { return m_symbols.get(name); }

void ListingDocumentTypeImpl::replace(address_t address, type_t type)
{
    this->remove(address, type);
    this->insert(address, type);
}

void ListingDocumentTypeImpl::removeAt(size_t idx)
{
    this->notify(idx, ListingDocumentChangedEventArgs::Removed);
    ListingItem item = m_items.at(idx);
    m_items.erase(idx);

    switch(item.type)
    {
        case ListingItem::InstructionItem: m_instructions.erase(item.address); break;
        case ListingItem::SegmentItem: m_segments.erase(item.address); break;
        case ListingItem::SeparatorItem: m_separators.erase(item.address); break;

        case ListingItem::SymbolItem:
            if(!m_functions.contains(item.address)) m_symbols.erase(item.address); // Don't delete functions
            break;

        case ListingItem::FunctionItem:
            m_functions.erase(item.address);
            m_symbols.erase(item.address);
            this->remove(item.address, ListingItem::EmptyItem);
            break;

        case ListingItem::TypeItem:
            this->remove(item.address, ListingItem::EmptyItem);
            break;

        default: break;
    }
}

const FunctionGraph* ListingDocumentTypeImpl::graph(address_t address) const { return m_functions.graph(address); }
FunctionGraph* ListingDocumentTypeImpl::graph(address_t address) { return m_functions.graph(address);  }
void ListingDocumentTypeImpl::graph(address_t address, FunctionGraph* graph) { m_functions.graph(address, graph); }

void ListingDocumentTypeImpl::segmentCoverage(address_t address, size_t coverage)
{
    size_t idx = m_segments.indexOf(address);

    if(idx == REDasm::npos)
    {
        r_ctx->problem("Cannot find segment @ " + String::hex(address));
        return;
    }

    this->segmentCoverageAt(idx, coverage);
}

void ListingDocumentTypeImpl::segmentCoverageAt(size_t idx, size_t coverage)
{
    Segment* segment = m_segments.at(idx);

    if((coverage == REDasm::npos) || (segment->coveragebytes == REDasm::npos)) segment->coveragebytes = coverage;
    else segment->coveragebytes += coverage;
}

void ListingDocumentTypeImpl::invalidateGraphs()
{
    while(!m_separators.empty())
        this->remove(*m_separators.begin(), ListingItem::SeparatorItem);

    m_functions.invalidateGraphs();
}

void ListingDocumentTypeImpl::remove(address_t address, type_t type)
{
    size_t idx = m_items.indexOf(address, type);
    if(idx == REDasm::npos) return;
    this->removeAt(idx);
}

void ListingDocumentTypeImpl::createSymbol(address_t address, const String& name, type_t type, flag_t flags, tag_t tag)
{
    if(r_disasm->needsWeak()) flags |= Symbol::F_Weak;

    if(name.empty()) m_symbols.create(address, type, flags, tag);
    else m_symbols.create(address, name, type, flags, tag);
}

bool ListingDocumentTypeImpl::canSymbolizeAddress(address_t address, type_t type, flag_t flags) const
{
    if(!m_segments.find(address)) return false; // Ignore out of segment addresses
    if(r_disasm->needsWeak()) flags |= Symbol::F_Weak;

    const BlockItem* bi = m_blocks.find(address);
    if((flags & Symbol::F_Weak) && bi->typeIs(BlockItem::T_Code)) return false;

    const Symbol* symbol = this->symbol(address);
    if(!symbol) return true;

    if(symbol->type > type) return false;
    if((symbol->type == type) && (!symbol->isWeak() && (flags & Symbol::F_Weak))) return false;
    return true;
}

void ListingDocumentTypeImpl::onBlockInserted(const EventArgs* e)
{
    using BlockEventArgs = ValueEventArgs<BlockItem*>;
    const BlockItem* bi = static_cast<const BlockEventArgs*>(e)->value;

    switch(bi->type)
    {
        case BlockItem::T_Unexplored: this->insert(bi->start, ListingItem::UnexploredItem); break;
        //case BlockItem::Data: this->insert(bi->start, ListingItem::SymbolItem); break;         // Don't add SymbolItem automatically
        case BlockItem::T_Code: this->insert(bi->start, ListingItem::InstructionItem); break;
        default: break;
    }
}

void ListingDocumentTypeImpl::onBlockErased(const EventArgs* e)
{
    using BlockEventArgs = ValueEventArgs<BlockItem*>;
    const BlockItem* bi = static_cast<const BlockEventArgs*>(e)->value;

    switch(bi->type)
    {
        case BlockItem::T_Unexplored: this->remove(bi->start, ListingItem::UnexploredItem); break;
        case BlockItem::T_Data: this->remove(bi->start, ListingItem::SymbolItem); break;
        case BlockItem::T_Code: this->remove(bi->start, ListingItem::InstructionItem); break;
    }
}

} // namespace REDasm
