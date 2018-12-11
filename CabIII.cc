/*
	CabIII.cc
	
	Copyright 2002-18 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Loudspeaker cabinet emulation through IIR filtering.

*/
/*
	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 3
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

#include "basics.h"

#include "dsp/RBJ.h"
#include "Cabinet.h"
#include "Descriptor.h"

#include "CabIIIModels.h"

void
CabinetIII::init()
{
	models = fs > 46000 ? allmodels : allmodels + CabIIIModels;

	h = 0;
	model = -1;
}

void
CabinetIII::switch_model (int m)
{
	model = m;
	if (fs > 46000) m += CabIIIModels;

	m %= (2*CabIIIModels);
	a = models[m].a;
	b = models[m].b;
	gain = models[m].gain*db2lin(getport(2));

	memset(x, 0, sizeof(x));
	memset(y, 0, sizeof(y));
}

void
CabinetIII::activate()
{
	/* ensure model switch at next run() */
	model = -1;
}

void
CabinetIII::cycle (uint frames)
{
	int m = (int) getport(0) + CabIIIModels*((int) getport(1));
	if (m != model) switch_model (m);

	sample_t g = models[model].gain*db2lin (getport(2));
	double gf = pow (g/gain, 1/(double) frames);

	sample_t * s = ports[3];
	sample_t * d = ports[4];

	for (uint i = 0; i < frames; ++i)
	{
		register cabinet_float acc = s[i] + normal;
		
		x[h] = acc;
		acc *= a[0];
		for(int j=1, z=h-1; j<32; --z,++j)
		{
			z &= 31;
			acc += a[j]*x[z];
			acc += b[j]*y[z];
		}
		y[h] = acc;
		d[i] = gain*acc;

		h = (h+1) & 31;
		gain *= gf;
	}
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
CabinetIII::port_info [] =
{
	{ "model", CTRL_IN, {INTEGER | DEFAULT_1, 0, CabIIIModels-1}, CabIIIModelDict }, 
	{ "alt", CTRL_IN, {INTEGER | DEFAULT_0, 0, 1}, "{0:'off',1:'on',}" }, 
	{ "gain (dB)", CTRL_IN | GROUP, {DEFAULT_0, -24, 24} }, 

	{ "in", INPUT | AUDIO },
	{ "out", OUTPUT | AUDIO }
};

/* //////////////////////////////////////////////////////////////////////// */

template <> void
Descriptor<CabinetIII>::setup()
{
	Label = "CabinetIII";
	Name = CAPS "CabinetIII - Simplistic loudspeaker cabinet emulation";
	autogen();
}

