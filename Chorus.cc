/*
	Chorus.cc
	
	Copyright 2004-13 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Chorus units.
	
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

#include "Chorus.h"
#include "Descriptor.h"

void
ChorusI::activate()
{
	setrate (getport(0));
	time = 0;
	width = 0;
	delay.reset();
}

void
ChorusI::setrate (float r)
{
	if (r == rate) return;
	rate = r;
	lfo.sine.set_f (rate, fs, lfo.sine.get_phase());
}

template <yield_func_t F>
void
ChorusI::one_cycle (int frames)
{
	float one_over_n = 1/(float)frames;
	float ms = fs*.001;

	float t = time;
	time = getport(0)*ms;
	float dt = (time - t)*one_over_n;

	float w = width;
	width = getport(1)*ms;
	/* clamp, or we need future samples from the delay line */
	if (width >= t - 3) width = t - 3;
	float dw = (width - w) * one_over_n;

	setrate (getport(2));
	
	float blend = getport(3);
	float ff = getport(4);
	float fb = getport(5);

	sample_t * s = ports[6];
	sample_t * d = ports[7];

	DSP::FPTruncateMode truncate;

	for (int i = 0; i < frames; ++i)
	{
		sample_t x = s[i];

		int ti;
		fistp (t, ti);
		x -= fb*delay[ti];

		delay.put (x + normal);

		x = blend*x + ff*delay.get_cubic (t + w*lfo.sine.get());

		F (d, i, x, adding_gain);

		t += dt;
		w += dw;
	}
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
ChorusI::port_info [] =
{
	{ "t (ms)", CTRL_IN, {LOG | DEFAULT_MID, 2.5, 40} }, 
	{ "width (ms)", CTRL_IN, {DEFAULT_LOW, .5, 10} }, 

	{ "rate (Hz)", CTRL_IN | GROUP, {DEFAULT_LOW, 0.001, 5} }, 

	{ "blend", CTRL_IN | GROUP, {DEFAULT_1, 0, 1} }, 
	{ "feedforward", CTRL_IN, {DEFAULT_LOW, 0, 1} }, 
	{ "feedback", CTRL_IN, {DEFAULT_0, 0, 1} }, 

	{ "in", INPUT | AUDIO }, 
	{ "out", OUTPUT | AUDIO }
};

template <> void
Descriptor<ChorusI>::setup()
{
	Label = "ChorusI";

	Name = CAPS "ChorusI - Mono chorus/flanger";
	Maker = "Tim Goetze <tim@quitte.de>";
	Copyright = "GPL, 2004-13";

	/* fill port info and vtable */
	autogen();
}


