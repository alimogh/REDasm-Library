#pragma once

#include "../../../disassembler/listing/listingdocument.h"
#include "../../../types/object.h"

namespace REDasm {

class Disassembler;
class PrinterImpl;

class Printer: public Object
{
    REDASM_OBJECT(Printer)
    PIMPL_DECLARE_P(Printer)
    PIMPL_DECLARE_PRIVATE(Printer)

    public:
        typedef std::function<void(const Operand*, const String&, const String&)> OpCallback;
        typedef std::function<void(const Symbol*, const String&)> SymbolCallback;
        typedef std::function<void(const String&, const String&, const String&)> FunctionCallback;
        typedef std::function<void(const String&)> LineCallback;

    protected:
        Printer(PrinterImpl* p);

    public:
        Printer(Disassembler* disassembler);
        virtual ~Printer() = default;
        Disassembler* disassembler() const;
        const ListingDocument& document() const;
        String symbol(const Symbol *symbol) const;
        String out(const CachedInstruction& instruction) const;

    public:
        virtual void segment(const Segment* segment, const LineCallback &segmentfunc);
        virtual void function(const Symbol *symbol, const FunctionCallback &functionfunc);
        virtual void symbol(const Symbol *symbol, const SymbolCallback& symbolfunc) const;
        virtual String out(const CachedInstruction& instruction, const OpCallback& opfunc) const;

    public: // Operand privitives
        virtual String reg(const RegisterOperand &regop) const;
        virtual String disp(const Operand *operand) const;
        virtual String loc(const Operand* operand) const;
        virtual String mem(const Operand* operand) const;
        virtual String imm(const Operand* operand) const;
        virtual String size(const Operand* operand) const;
};

} // namespace REDasm
