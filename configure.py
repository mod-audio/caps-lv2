#! /usr/bin/env python
import os

CFLAGS = []
ARCH = []

def dude_we_think_so_different():
	try: return 'Darwin' == os.popen ('uname -s').read().strip()
	except: return 0
OSX = dude_we_think_so_different()
if OSX:
	OSX_LDFLAGS = "-bundle -undefined suppress -flat_namespace"
	ARCH += ("-arch","i386","-arch","x86_64")
	import ctypes as C
	libc = C.CDLL("libc.dylib")
	def osx_query(k):
		size = C.c_uint(0)
		libc.sysctlbyname(k,None,C.byref(size),None,0)
		buf = C.create_string_buffer(size.value)
		libc.sysctlbyname(k,buf,C.byref(size),None,0)
		return buf.value

def we_have_sse():
	if OSX: return osx_query("hw.optional.sse2")
	try: return 'sse' in open ('/proc/cpuinfo').read().split()
	except: return 0
def we_have_ssse3():
	if OSX: return osx_query("hw.optional.sse3")
	try: return 'ssse3' in open ('/proc/cpuinfo').read().split()
	except: return 0
	
def store():
	f = open ('defines.make', 'w')
	f.write ("_CFLAGS=" + ' '.join (CFLAGS) + "\n")
	f.write ("ARCH=" + ' '.join (ARCH) + "\n")
	if OSX:
		f.write ("_LDFLAGS=" + OSX_LDFLAGS + "\n")
		f.write ("STRIP = echo\n")
	os.system ("make clean")

if __name__ == '__main__':
	if we_have_sse(): CFLAGS += ('-msse', '-mfpmath=sse')
	if we_have_ssse3(): CFLAGS += ('-msse3',)
	store()
