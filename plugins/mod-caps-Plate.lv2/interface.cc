
#include "version.h"
#include "basics.h"

#include "Cabinet.h"
#include "Chorus.h"
#include "Phaser.h"
#include "Sin.h"
#include "Fractals.h"
#include "Reverb.h"
#include "Compress.h"
#include "Click.h"
#include "Eq.h"
#include "Saturate.h"
#include "White.h"
#include "AutoFilter.h"
#include "Amp.h"
#include "Pan.h"
#include "Scape.h"
#include "ToneStack.h"
#include "Noisegate.h"

#include "Descriptor.h"

LV2_SYMBOL_EXPORT
const LV2_Descriptor *
lv2_descriptor(uint32_t i)
{
    static const Descriptor<Plate> lv2_descriptor(CAPS_URI "Plate");
    return i == 0 ? static_cast<const LV2_Descriptor*>(&lv2_descriptor) : 0;
}
