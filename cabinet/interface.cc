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

#include "Cabinet.h"

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
    CAPS_URI "CabinetI",
    Descriptor<CabinetI>::_instantiate,
    Descriptor<CabinetI>::_connect_port,
    Descriptor<CabinetI>::_activate,
    Descriptor<CabinetI>::_run,
    Descriptor<CabinetI>::_deactivate,
    Descriptor<CabinetI>::_cleanup,
    Descriptor<CabinetI>::_extension_data
    },
    {
    CAPS_URI "CabinetII",
    Descriptor<CabinetII>::_instantiate,
    Descriptor<CabinetII>::_connect_port,
    Descriptor<CabinetII>::_activate,
    Descriptor<CabinetII>::_run,
    Descriptor<CabinetII>::_deactivate,
    Descriptor<CabinetII>::_cleanup,
    Descriptor<CabinetII>::_extension_data
    }
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
    if (index < N) return &Descriptors[index];
    else return NULL;
}
