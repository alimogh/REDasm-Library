#ifndef CHIP8_LOADER_H
#define CHIP8_LOADER_H

#include "../../plugins/plugins.h"

namespace REDasm {

class CHIP8Loader: public LoaderPluginB
{
    PLUGIN_NAME("CHIP-8 ROM")

    public:
        CHIP8Loader(AbstractBuffer* buffer);
        virtual std::string assembler() const;
        virtual u32 bits() const;
        virtual void load();
};

DECLARE_LOADER_PLUGIN(CHIP8Loader, chip8)

} // namespace REDasm

#endif // CHIP8_LOADER_H