/*
	Descriptor.h
	
	Copyright 2004-13 Tim Goetze <tim@quitte.de>
	
	http://quitte.de/dsp/

	Creating a LADSPA_Descriptor for a CAPS plugin via a C++ template,
	saving a virtual function call compared to the usual method used
	for C++ plugins in a C context.

	Descriptor<P> expects P to declare some common methods, like init(),
	activate() etc, plus a static port_info[] and LADSPA_Data * ports[]
	and adding_gain.  (P should derive from Plugin, too.)
 
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

#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#ifdef __SSE__
#include <xmmintrin.h>
#endif
#ifdef __SSE3__
#include <pmmintrin.h>
#endif

#include <lv2.h>

/* not used */
inline void
processor_specific_denormal_measures()
{
	#ifdef __SSE3__
	/* this one works reliably on a 6600 Core2 */
	_MM_SET_DENORMALS_ZERO_MODE (_MM_DENORMALS_ZERO_ON);
	#endif

	#ifdef __SSE__
	/* this one doesn't ... */
	_MM_SET_FLUSH_ZERO_MODE (_MM_FLUSH_ZERO_ON);
	#endif
}

/* common stub for Descriptor makes it possible to delete() without special-
 * casing for every plugin class.
 */
class DescriptorStub
: public LADSPA_Descriptor, public LV2_Descriptor
{
	public:
		DescriptorStub()
			{
				PortCount = 0;
				URI = 0;
			}

		~DescriptorStub()
			{
				if (PortCount)
				{
					delete [] PortNames;
					delete [] PortDescriptors;
					delete [] PortRangeHints;
				}
			}
};

template <class T>
class Descriptor
: public DescriptorStub
{
	public:
		LADSPA_PortRangeHint * ranges;

	public:
		Descriptor (uint id) { UniqueID = id; setup(); }
		Descriptor (const char *uri) { URI = uri; setup(); }

		void setup(); 
		/* setup() is in the plugin's .cc implementation file because it needs 
		 * access to the port_info implementation, instantiating the following
		 * function: */
		void autogen() 
			{
				Properties = HARD_RT;
				PortCount = (sizeof (T::port_info) / sizeof (PortInfo));

				ImplementationData = T::port_info;

				/* convert PortInfo members to Descriptor properties */
				const char ** names = new const char * [PortCount];
				PortNames = names;

				LADSPA_PortDescriptor * desc = new LADSPA_PortDescriptor [PortCount];
				PortDescriptors = desc;

				ranges = new LADSPA_PortRangeHint [PortCount];
				PortRangeHints = ranges;

				for (int i = 0; i < (int) PortCount; ++i)
				{
					names[i] = T::port_info[i].name;
					desc[i] = T::port_info[i].descriptor;
					ranges[i] = T::port_info[i].range;
					if (desc[i] & INPUT)
						ranges[i].HintDescriptor |= BOUNDED;
				}
				
				/* Descriptor vtable */
				LADSPA_Descriptor::instantiate = _instantiate;
				LADSPA_Descriptor::connect_port = _connect_port;
				LADSPA_Descriptor::activate = _activate;
				LADSPA_Descriptor::run = _run;
				LADSPA_Descriptor::run_adding = _run_adding;
				LADSPA_Descriptor::set_run_adding_gain = _set_run_adding_gain;
				LADSPA_Descriptor::deactivate = 0;
				LADSPA_Descriptor::cleanup = _cleanup;

				LV2_Descriptor::instantiate = _instantiate_lv2;
				LV2_Descriptor::connect_port = _connect_port_lv2;
				LV2_Descriptor::activate = _activate_lv2;
				LV2_Descriptor::run = _run_lv2;
				LV2_Descriptor::deactivate = 0;
				LV2_Descriptor::cleanup = _cleanup_lv2;
			}

		static LADSPA_Handle _instantiate (
				const struct _LADSPA_Descriptor * d, ulong fs)
			{ 
				T * plugin = new T();

				LADSPA_PortRangeHint * ranges = ((Descriptor *) d)->ranges;
				plugin->ranges = ranges;

				int n = (int) d->PortCount;
				plugin->ports = new sample_t * [n];
				/* connect to lower bound as a safety measure */
				for (int i = 0; i < n; ++i)
					plugin->ports[i] = &(ranges[i].LowerBound);

				plugin->fs = fs;
				plugin->over_fs = 1./fs;
				plugin->normal = NOISE_FLOOR;
				plugin->init();

				return plugin;
			}
		
		static void _connect_port (LADSPA_Handle h, ulong i, LADSPA_Data * p)
			{ 
				((T *) h)->ports[i] = p;
			}

		static void _activate (LADSPA_Handle h)
			{
				T * plugin = (T *) h;

				plugin->first_run = 1;

				/* since none of the plugins do any RT-critical work in 
				 * activate(), it's safe to defer the actual call into
				 * the first run() after the host called activate().
				 * 
				 * It's the simplest way to prevent a parameter smoothing sweep
				 * in the first audio block after activation.
				 *
				 * While it would be preferable to set up the plugin's internal
				 * state from the current set of parameters, ladspa.h allows hosts to
				 * call activate() without even having connected the inputs, so that
				 * is out of the question.
				plugin->activate();
				 */
			}

		static void _run (LADSPA_Handle h, ulong n)
			{
				if (!n) return;

				T * plugin = (T *) h;

				/* If this is the first audio block after activation, 
				 * initialize the plugin from the current set of parameters. */
				if (plugin->first_run)
				{
					plugin->activate();
					plugin->first_run = 0;
				}

				plugin->run (n);
				plugin->normal = -plugin->normal;
			}
		
		static void _run_adding (LADSPA_Handle h, ulong n)
			{
				if (!n) return;

				T * plugin = (T *) h;

				/* If this is the first audio block after activation, 
				 * initialize the plugin from the current set of parameters. */
				if (plugin->first_run)
				{
					plugin->activate();
					plugin->first_run = 0;
				}

				plugin->run_adding (n);
				plugin->normal = -plugin->normal;
			}
		
		static void _set_run_adding_gain (LADSPA_Handle h, LADSPA_Data g)
			{
				T * plugin = (T *) h;

				plugin->adding_gain = g;
			}

		static void _cleanup (LADSPA_Handle h)
			{
				T * plugin = (T *) h;

				delete [] plugin->ports;
				delete plugin;
			}

		/* LV2 */
		static LV2_Handle _instantiate_lv2 (
				const LV2_Descriptor *d, double fs, const char *bundle_path, const LV2_Feature *const *features)
			{
				T * plugin = new T();

				LADSPA_PortRangeHint * ranges = ((Descriptor *) d)->ranges;
				plugin->ranges = ranges;

				int n = 32; /* FIXME: hardcoded port numbers */
				plugin->ports = new sample_t * [n];

				plugin->fs = fs;
				plugin->over_fs = 1./fs;
				plugin->normal = NOISE_FLOOR;
				plugin->init();

				return plugin;
			}

		static void _connect_port_lv2 (LV2_Handle h, uint32_t i, void * p)
			{
				((T *) h)->ports[i] = (sample_t *) p;
			}

		static void _activate_lv2 (LV2_Handle h)
			{
				_activate(h);
			}

		static void _run_lv2 (LV2_Handle h, uint32_t n)
			{
				_run(h, n);
			}

		static void _cleanup_lv2 (LV2_Handle h)
			{
				_cleanup(h);
			}
};

#endif /* DESCRIPTOR_H */
