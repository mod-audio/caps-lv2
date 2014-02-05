/*
	dsp/SVF.h
	
	Copyright 2002-12 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	SVF*: State-variable filter in Chamberlin topology,
		supports f and Q sweeps.

	StackedSVF: building ladders from the above.

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
/*
  Based on this music-dsp entry:

	State Variable Filter (Double Sampled, Stable)
	Type : 2 Pole Low, High, Band, Notch and Peaking
	References :Posted by Andrew Simper

	Notes :
	Thanks to Laurent de Soras for the stability limit
	and Steffan Diedrichsen for the correct notch output.

	Code :
	input  = input buffer;
	output = output buffer;
	fs     = sampling frequency;
	fc     = cutoff frequency normally something like:
					 440.0*pow(2.0, (midi_note - 69.0)/12.0);
	res    = resonance 0 to 1;
	drive  = internal distortion 0 to 0.1
	freq   = MIN(0.25, 2.0*sin(PI*fc/(fs*2)));  // the fs*2 is because it's double sampled
	damp   = MIN(2.0*(1.0 - pow(res, 0.25)), MIN(2.0, 2.0/freq - freq*0.5));
	notch  = notch output
	low    = low pass output
	high   = high pass output
	band   = band pass output
	peak   = peaking output = low - high
	-- 
	double sampled svf loop:
	for (i=0; i<numSamples; i++)
	{
		in    = input[i];
		notch = in - damp*band;
		low   = low + freq*band;
		high  = notch - low;
		band  = freq*high + band - drive*band*band*band;
		out   = 0.5*(notch or low or high or band or peak);
		notch = in - damp*band;
		low   = low + freq*band;
		high  = notch - low;
		band  = freq*high + band - drive*band*band*band;
		out  += 0.5*(same out as above);
		output[i] = out;
	}
*/
#ifndef DSP_SVF_H
#define DSP_SVF_H

namespace DSP {

/* oversampling only makes sense for LP */
template <int Oversample>
class SVFI
{
	protected:
		/* loop parameters */
		sample_t f, q, qnorm;
		
		/* outputs (peak and notch left out) */
		sample_t lo, band, hi;
		sample_t * out;

	public:
		/* the type of filtering to do. */
		enum {
			Low = 0,
			Band = 1,
			High = 2
		};

		SVFI()
			{
				set_out (Low);
				set_f_Q (.1, .1);
			}
		
		void reset()
			{
				hi = band = lo = 0;
			}

		void set_f_Q (double fc, double Q)
			{
				/* this is a very tight limit */
				f = min (.25, 2 * sin (M_PI * fc / Oversample));

				q = 2 * cos (pow (Q, .1) * M_PI * .5);
				q = min (q, min (2., 2 / f - f * .5));
				qnorm = sqrt (fabs (q) / 2. + .001);
			}

		void set_out (int o)
			{
				if (o == Low)
					out = &lo;
				else if (o == Band)
					out = &band;
				else
					out = &hi;
			}

		void one_cycle (sample_t * s, int frames)
			{
				for (int i = 0; i < frames; ++i)
					s[i] = process (s[i]);
			}

		sample_t process (sample_t x)
			{
				x = qnorm * x;

				for (int pass = 0; pass < Oversample; ++pass)
				{
					hi = x - lo - q * band;
					band += f * hi;
					lo += f * band;

					/* zero padding */
					x = 0;
				}

				return *out;
			}
};

/* //////////////////////////////////////////////////////////////////////// */

/*
	Alternative SVF implementation based on (this is the corrected version):

	From andy@cytomic.com Mon May 23 21:47:40 2011
	Subject: Re: [music-dsp] Trapezoidal and other integration methods applied tomusical resonant filters
	From: Andrew Simper <andy@cytomic.com>
	To: A discussion list for music-related DSP <music-dsp@music.columbia.edu>
	Reply-To: A discussion list for music-related DSP <music-dsp@music.columbia.edu>
	Date: Tue, 17 May 2011 17:48:27 +1000

	Hi Vadim,

	I want to thank you for your excellent paper "Generation of
	bandlimited sync transitions for sine waveforms", which I feel is the
	most thorough coverage in a paper of the method I use in Synth Squad,
	which I call "corrective grains". You also outline in that paper
	another couple of methods which I didn't even know existed, so I thank
	you for showing me some interesting new ways to approach the problem
	of band limiting transitions in waveforms.

	I'm not sure what a "non-zero impedance approach" is, but standard
	circuit simulation stuff handles any number of arbitrary impedance
	devices connected in any topology, so I'm happy to not try
	re-inventing anything since I can apply these techniques in a brain
	dead crank the handle type way and get great results for both the
	linear and non-linear cases.

	Thanks for letting me know about all the papers you know of, it sounds
	like I should get on and publish something as it isn't really covered
	in detail anywhere yet. As a start below I have included the
	difference equation for the linear trapezoidal svf:

	init:
	v1 = v2 = 0;
	v0z = v1z = v2z = 0;

	process:
	g = tan (pi * cutoff / samplerate);
	k = damping factor (typically in the range 2 to 0);
	v1z = v1;
	v2z = v2;
	v0 = input;
	v1 = v1z + g * (v0 + v0z - 2*(g + k)*v1z - 2*v2z) / (1 + g*(g + k));
	v2 = v2z + g * (v1 + v1z);
	v0z = v0;

	outputs (the same as the analog circuit):
	band = v1;
	low = v2;
	high = v0 - k*v1 - v2;
	notch = high + low;
	peak = high - low;


	Andy
	--
	cytomic - sound music software
	skype: andrewsimper
*/

class SVFII
{
	public:
		sample_t v[3];
		sample_t k, g, c1, c2;
		int out;

	public:
		SVFII() { out = 0; reset(); }

		void set_out (int o) { out = 2-(o&1); }

		void reset()
			{
				for (uint i=0; i < 3; ++i)
					v[i] = 0;
			}

		void set_f_Q (sample_t fc, sample_t q)
			{
				k = (1-.99*q);
				g = tan (fc*M_PI);
				c1 = 2*(g+k);
				c2 = g/(1 + g*(g+k));
			}

		sample_t process (sample_t x) 
			{ _process (x); return v[out]; }

		void _process (sample_t x)
			{
				sample_t v1 = v[1] + c2*(x + v[0] - c1*v[1] - 2*v[2]);
				sample_t v2 = v[2] + g*(v1 + v[1]);
				v[0] = x;
				v[1] = v1;
				v[2] = v2;
			}

		/*
			outputs (the same as the analog circuit):
			band = v1;
			low = v2;
			high = v0 - k*v1 - v2;
			notch = high + low;
			peak = high - low;
		*/

		sample_t process_lp (sample_t x) 
			{ _process (x); return v[2]; }
		sample_t process_bp (sample_t x) 
			{ _process (x); return v[1]; }
		sample_t process_hp (sample_t x) 
			{ _process (x); return v[0] - k*v[1] - v[2]; }
};

/* //////////////////////////////////////////////////////////////////////// */

template <int N, class SVF>
class StackedSVF
{
	public:
		SVF svf[N];

		void reset()
			{
				for (int i = 0; i < N; ++i)
					svf[i].reset();
			}

		void set_out (int out)
			{
				for (int i = 0; i < N; ++i)
					svf[i].set_out (out);
			}

		void set_f_Q (double f, double Q)
			{
				for (int i = 0; i < N; ++i)
					svf[i].set_f_Q (f, Q);
			}

		sample_t process (sample_t x)
			{
				for (int i = 0; i < N; ++i)
					x = svf[i].process (x);
				return x;
			}

	template<clip_func_t clip>
		sample_t process (sample_t x, sample_t g)
			{
				for (int i = 0; i < N; ++i)
					x = clip (svf[i].process (g*x));
				return x;
			}
};

} /* namespace DSP */

#endif /* DSP_SVF_H */
