Ok, I just uploaded it. 
 http://myweb.tiscali.co.uk/simonhall/cyg-profile.c 
 http://myweb.tiscali.co.uk/simonhall/cyg-profile.h 
 http://myweb.tiscali.co.uk/simonhall/resolve.pl 
 
You compile your (ARM-only) regular code with -finstrument-functions
-mpoke-function-name. Don't compile the files which handle disk access
or anything related to the profiling functions, otherwise you'll get
stuck in a loop.
 
 - Initialise the profiler with cygprofile_begin(). 
 - Start profiling with cygprofile_enable(). 
 - Stop profiling with cygprofile_disable(). 
 - Write the results to disk with cygprofile_end(). 
 - Parse the results (on the PC) with 
   ./resolve.pl squake /cygdrive/k/CYGLOG.TXT > /cygdrive/k/sorted.csv
   ...or something like that :-) 
 
These files are tailored for Quake btw, so they won't compile outa the
box. I normally run my profiling for 700 frames (nearly the length of
the Necropolis demo) then write everything to disk - that's why
there's some divide-by-700s in the Perl script!
 
Oh and thanks go out to the people who wrote the original cyg-profile
stuff (before I rewrote all the juicy bits)

-------------------------------------------------------------------------------

Yeah, I'm not with tiscali any more so I'm not surprised my web space
doesn't work!

Here's it hosted on drunkencoders (thanks again to dovoto): 

 - http://quake.drunkencoders.com/profiler/resolve (perl script) 
 - http://quake.drunkencoders.com/profiler/cyg-profile.c 
 - http://quake.drunkencoders.com/profiler/cyg-profile.h 
 
Instructions per the post a few above this. It's still Quakified, but
shouldn't be hard to convert to whatever you want to run it with.  NEW
STUFF: it now works in THUMB mode, so you're no longer forced to use
ARM mode to get it to work.

--------------------------------------------------------------------------------

I stuck the files for this on the quake web site many moons ago (and
put a link on gbadev, too)

 - http://quake.drunkencoders.com/profiler/cyg-profile.c 
 - http://quake.drunkencoders.com/profiler/cyg-profile.h 
 - http://quake.drunkencoders.com/profiler/resolve (rename this file
   to resolve.pl once you've downloaded it) 
 
To use it, again I've put instructions in another thread on here,
compile with -finstrument-functions -mpoke-function-name on the source
that you want to instrument (don't do it to interrupt handlers, or
stuff that does file access cos you'll get it stuck in a loop and run
out of stack).  Call cygprofile_begin() to set up the data structures,
then call cygprofile_enable() to start instrumentation and then when
you want to stop profiling your code (eg when you quit your program)
call cygprofile_end(). Job done.
 
You do not need to do this for each function you want to profile - it
will instrument every function called until you call cygprofile_end.
 
(btw you'll want to wiggle the code slightly to call your own
file-writing and opening functions, plus you need to provide the
number of hblanks since program start in 'hblanks')

-------------------------------------------------------------------------------

For å unngå instrumentering av enkelte funksjoner:

__attribute__ ((no_instrument_function))

