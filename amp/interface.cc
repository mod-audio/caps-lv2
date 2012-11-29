/*
  interface.cc

    Copyright 2004-11 Tim Goetze <tim@quitte.de>

    http://quitte.de/dsp/

    LADSPA descriptor factory, host interface.

*/
/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
    02111-1307, USA or point your web browser to http://www.gnu.org.
*/
/*
    LADSPA ID ranges 1761 - 1800 and 2581 - 2660
    (2541 - 2580 donated to artemio@kdemail.net)
*/

#include <sys/time.h>

#include "basics.h"

#include "Amp.h"

#include "Descriptor.h"


#define N   4

static inline void
seed()
{
    static struct timeval tv;
  gettimeofday (&tv, 0);

    srandom (tv.tv_sec ^ tv.tv_usec);
}

#define CAPS_URI "http://portalmod.com/plugins/caps/"
static const LV2_Descriptor Descriptors[] = {
    {
    CAPS_URI "AmpIII",
    Descriptor<AmpIII>::_instantiate,
    Descriptor<AmpIII>::_connect_port,
    Descriptor<AmpIII>::_activate,
    Descriptor<AmpIII>::_run,
    Descriptor<AmpIII>::_deactivate,
    Descriptor<AmpIII>::_cleanup,
    Descriptor<AmpIII>::_extension_data
    },
    {
    CAPS_URI "AmpIV",
    Descriptor<AmpIV>::_instantiate,
    Descriptor<AmpIV>::_connect_port,
    Descriptor<AmpIV>::_activate,
    Descriptor<AmpIV>::_run,
    Descriptor<AmpIV>::_deactivate,
    Descriptor<AmpIV>::_cleanup,
    Descriptor<AmpIV>::_extension_data
    },
    {
    CAPS_URI "AmpV",
    Descriptor<AmpV>::_instantiate,
    Descriptor<AmpV>::_connect_port,
    Descriptor<AmpV>::_activate,
    Descriptor<AmpV>::_run,
    Descriptor<AmpV>::_deactivate,
    Descriptor<AmpV>::_cleanup,
    Descriptor<AmpV>::_extension_data
    },
    {
    CAPS_URI "AmpVTS",
    Descriptor<AmpVTS>::_instantiate,
    Descriptor<AmpVTS>::_connect_port,
    Descriptor<AmpVTS>::_activate,
    Descriptor<AmpVTS>::_run,
    Descriptor<AmpVTS>::_deactivate,
    Descriptor<AmpVTS>::_cleanup,
    Descriptor<AmpVTS>::_extension_data
    }
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
    if (index < N) return &Descriptors[index];
    else return NULL;
}
