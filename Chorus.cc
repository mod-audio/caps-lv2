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

template <yield_func_t F>
void
ChorusI::one_cycle (int frames)
{
    sample_t * s = ports[0];

    double one_over_n = 1 / (double) frames;
    double ms = .001 * fs;

    double t = time;
    time = getport(1) * ms;
    double dt = (time - t) * one_over_n;

    double w = width;
    width = getport(2) * ms;
    /* clamp, or we need future samples from the delay line */
    if (width >= t - 3) width = t - 3;
    double dw = (width - w) * one_over_n;

    if (rate != *ports[3])
        lfo.set_f (max (rate = getport(3), .000001), fs, lfo.get_phase());

    double blend = getport(4);
    double ff = getport(5);
    double fb = getport(6);

    sample_t * d = ports[7];

    DSP::FPTruncateMode truncate;

    for (int i = 0; i < frames; ++i)
    {
        sample_t x = s[i];

        /* truncate the feedback tap to integer, better quality for less
         * cycles (just a bit of zipper when changing 't', but it does sound
         * interesting) */
        int ti;
        fistp (t, ti);
        x -= fb * delay[ti];

        delay.put (x + normal);

#       if 0
        /* allpass delay sounds a little cleaner for a chorus
         * but sucks big time when flanging. */
        x = blend * x + ff * tap.get (delay, t + w * lfo.get());
#       elif 0
        /* linear interpolation */
        x = blend * x + ff * delay.get_at (t + w * lfo.get());
#       else
        /* cubic interpolation */
        x = blend * x + ff * delay.get_cubic (t + w * lfo.get());
#       endif

        F (d, i, x, adding_gain);

        t += dt;
        w += dw;
    }
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
ChorusI::port_info [] =
{
    { "in", INPUT | AUDIO },
    { "t (ms)", CTRL_IN, {BOUNDED | LOG | DEFAULT_LOW, 2.5, 40} },
    { "width (ms)", CTRL_IN, {BOUNDED | DEFAULT_1, .5, 10} },
    { "rate (Hz)", CTRL_IN | GROUP, {BOUNDED | DEFAULT_LOW, 0, 5} },
    { "blend", CTRL_IN, {BOUNDED | DEFAULT_1, 0, 1} },
    { "feedforward", CTRL_IN | GROUP, {BOUNDED | DEFAULT_LOW, 0, 1} },
    { "feedback", CTRL_IN, {BOUNDED | DEFAULT_0, 0, 1} },
    { "out", OUTPUT | AUDIO }
};

template <> void
Descriptor<ChorusI>::setup()
{
    Label = "ChorusI";

    #ifdef LADSPAFLAG
    Properties = HARD_RT;
    #endif


    Name = CAPS "ChorusI - Mono chorus/flanger";
    Maker = "Tim Goetze <tim@quitte.de>";
    Copyright = "GPL, 2004-13";

    /* fill port info and vtable */
    autogen();
}

/* //////////////////////////////////////////////////////////////////////// */

template <yield_func_t F>
void
ChorusII::cycle (uint frames)
{
    sample_t * s = ports[0];

    double over_n = 1 / (double) frames;
    double ms = .001 * fs;

    double t = time;
    time = getport(1) * ms;
    double dt = (time - t) * over_n;

    double w = width;
    width = getport(2) * ms;
    /* clamp, lest we need future samples from the delay line */
    if (width >= t - 3) width = t - 3;
    double dw = (width - w) * over_n;

    float r = getport(3);
    if (r != rate)
        set_rate (r);

    double blend = getport(4);
    double ff = getport(5);
    double fb = getport(6);

    sample_t * d = ports[7];

    DSP::FPTruncateMode truncate;

    for (uint i = 0; i < frames; ++i)
    {
        sample_t x = s[i];

        x = hp.process (x+normal);
        x -= fb * delay.get_cubic (t);

        delay.put (x);

        double a = 0;
        for (int j = 0; j < Taps; ++j)
            a += taps[j].get (delay, t, w);

        x = blend * x + ff * a;

        F (d, i, x, adding_gain);

        t += dt;
        w += dw;
    }
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
ChorusII::port_info [] =
{
    { "in", INPUT | AUDIO },
    { "t (ms)", CTRL_IN, {LOG | DEFAULT_LOW, 2.5, 25} },
    { "width (ms)", CTRL_IN, {DEFAULT_LOW, .5, 10} },
    { "rate", CTRL_IN | GROUP, {DEFAULT_LOW, 0, 1} },
    { "blend", CTRL_IN, {DEFAULT_MID, 0, 1} },
    { "feedforward", CTRL_IN | GROUP, {DEFAULT_MID, 0, 1} },
    { "feedback", CTRL_IN, {DEFAULT_0, 0, 1} },
    { "out", OUTPUT | AUDIO }
};

template <> void
Descriptor<ChorusII>::setup()
{
    Label = "ChorusII";

    Name = CAPS "ChorusII - Multivoice chorus modulated by a fractal";
    Maker = "Tim Goetze <tim@quitte.de>";
    Copyright = "2004-12";

    /* fill port info and vtable */
    autogen();
}

/* //////////////////////////////////////////////////////////////////////// */

void
StereoChorusII::set_rate (sample_t r)
{
    rate = r;
    r *= FRACTAL_RATE * 44100 * over_fs;
    left.fractal.set_rate (r);
    right.fractal.set_rate (1.1*r);
    left.lfo_lp.set_f (.0001 * over_fs);
    right.lfo_lp.set_f (.0001 * over_fs);
}

template <yield_func_t F>
void
StereoChorusII::cycle (uint frames, int mode)
{
    double one_over_n = 1 / (double) frames;
    double ms = .001 * fs;

    double t = time;
    time = getport(0) * ms;
    double dt = (time - t) * one_over_n;

    double w = width;
    width = getport(1) * ms;
    /* clamp, lest we need future samples from the delay line */
    if (width >= t - 1) width = t - 1;
    double dw = (width - w) * one_over_n;

    set_rate (getport(2));

    double blend = getport(3);
    double ff = getport(4);
    double fb = getport(5);

    /* mode is Mono = 0 or Stereo = 1 */
    sample_t * sl = ports[6];
    sample_t * sr = ports[6+mode];

    sample_t * dl = ports[7+mode];
    sample_t * dr = ports[8+mode];

    /* ensure i386's fistp instruction truncates */
    DSP::FPTruncateMode truncate;

    for (uint i = 0; i < frames; ++i)
    {
        sample_t xl = sl[i], xr = sr[i], x = .5*(xl + xr);

        x = hp.process (x+normal);

        /* truncate the feedback tap to integer, better quality for less
         * cycles (just a bit of zipper when changing 't') */
        int ti;
        fistp (t, ti);
        x -= fb * delay[ti];

        delay.put (x);

        double m;
        m = left.lfo_lp.process (left.fractal.get());
        sample_t l = blend * xl + ff * delay.get_cubic (t + w * m);
        m = right.lfo_lp.process (right.fractal.get());
        sample_t r = blend * xr + ff * delay.get_cubic (t + w * m);

        F (dl, i, l, adding_gain);
        F (dr, i, r, adding_gain);

        t += dt;
        w += dw;
    }
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
StereoChorusII::port_info [] =
{
    { "t (ms)", CTRL_IN, {DEFAULT_LOW, 2.5, 25} },
    { "width (ms)", CTRL_IN, {DEFAULT_1, .5, 10} },
    { "rate", CTRL_IN | GROUP, {DEFAULT_LOW, 0, 1} },
    { "blend", CTRL_IN, {DEFAULT_LOW, 0, 1} },
    { "feedforward", CTRL_IN | GROUP, {DEFAULT_1, 0, 1} },
    { "feedback", CTRL_IN, {DEFAULT_MID, 0, 1} },
    { "in", INPUT | AUDIO },
    { "out.l", OUTPUT | AUDIO },
    { "out.r", OUTPUT | AUDIO }
};

template <> void
Descriptor<StereoChorusII>::setup()
{
    Label = "StereoChorusII";

    Name = CAPS "StereoChorusII - Stereo chorus modulated by a fractal";
    Maker = "Tim Goetze <tim@quitte.de>";
    Copyright = "2004-12";

    /* fill port info and vtable */
    autogen();
}

/* //////////////////////////////////////////////////////////////////////// */

PortInfo
StereoChorusII2x2::port_info [] =
{
    { "t (ms)", CTRL_IN, {DEFAULT_LOW, 2.5, 25} },
    { "width (ms)", CTRL_IN, {DEFAULT_1, .5, 10} },
    { "rate", CTRL_IN | GROUP, {DEFAULT_LOW, 0, 1} },
    { "blend", CTRL_IN, {DEFAULT_LOW, 0, 1} },
    { "feedforward", CTRL_IN | GROUP, {DEFAULT_1, 0, 1} },
    { "feedback", CTRL_IN, {DEFAULT_MID, 0, 1} },
    { "in.l", INPUT | AUDIO },
    { "in.r", INPUT | AUDIO },
    { "out.l", OUTPUT | AUDIO },
    { "out.r", OUTPUT | AUDIO }
};

template <> void
Descriptor<StereoChorusII2x2>::setup()
{
    Label = "StereoChorusII2x2";

    Name = CAPS "StereoChorusII2x2 - Stereo chorus modulated by a fractal";
    Maker = "Tim Goetze <tim@quitte.de>";
    Copyright = "2004-12";

    /* fill port info and vtable */
    autogen();
}


