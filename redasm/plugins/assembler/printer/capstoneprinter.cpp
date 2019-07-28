#include "capstoneprinter.h"
#include <impl/plugins/assembler/printer/capstoneprinter_impl.h>
#include "../../../context.h"

namespace REDasm {

CapstonePrinter::CapstonePrinter(): Printer(), m_pimpl_p(new CapstonePrinterImpl()) { }

String CapstonePrinter::reg(const RegisterOperand &regop) const
{
    if(regop.r <= 0)
    {
        r_ctx->problem("Unknown register with id " + String::number(regop.r));
        return "unkreg";
    }

    PIMPL_P(const CapstonePrinter);
    return cs_reg_name(p->m_handle, static_cast<unsigned int>(regop.r));
}

} // namespace REDasm