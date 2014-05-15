TRIGGER-STEREO 

Chuck Sites Jan 2005
chuck@cvip.uofl.edu

[Edited by Jasmine Langridge]


A. INTRODUCTION

   Trigger is an awesome game visually speaking and when I first saw
it, I wanted to get some 3D stereo hooks into it to really explore the
maps and scenes in 3D (Virtual Reality) like stereo graphics.  I can
now say that 3D stereo for Linux has finally arrived, and Trigger is
the first game that really demonstrates that capability thanks to Jaz
and Richard.

   I've been working with stereo vision for a long time starting with
Silicon Graphics Inc. (SGI) workstations, and for the past 10 Years, I've
longed for that level of capabilities with Linux.  Well, I think it
has finally arrived in hardware with the Nvidia line of Quadro
cards and the ATI FireGL cards.  These hardware accelerated graphics
boards perform better than some of the top of the line SGI workstations  
from my experience.  The down side is that the entry level stereo
capable cards like the entry level QuadroFX/500 (about the  equivalent
of a GeForceFX 5200)  cost about $300US! compared to $49 for the
GeForce.  Anyway, stereo with Linux in games is new, so I wanted to
put together some tips on how to get the best hardware accelerated
"quadbuffer" stereo performance with Linux.


B. SUPPORTED CARDS:

Nvidia Quadro series
ATI FireGL series  
Nvidia hardware hacked GeForce2 cards.  

  See: 'http://www.geocities.com/tnaw_xtennis/G-Quadro-2.htm'

  I've done this to a couple of cards and it really is not easy but 
  it does work.  I've lost some cards overtime where the screen 
  suddenly becomes fuzzy,  so I can't say the hardware fix is really
  stable. Some GeForce2 cards are not not documented and may not 
  be 'hack-able'.  Newer cards like the GeForce3, GeForce4, GeForceFX 
  and the new GeForce 6600 and 6800 are not hackable to the Quadro, 
  and I find it really frustrating that Nvidia does not support Stereo 
  in it's consumer level devices even though the chips are easily 
  capable of handling it.  Maybe someone can create a 'Softquadro' 
  solution (hacked device driver) for Linux like exists for Windows.

  SOFTQUADRO:
  Softquadro is widely used by students and professionals that want to
  get the most out of their NVIDIA GeForce(x) cards. The professional
  level cards by NVIDIA, such as Quadro, Quadro2 and Quadro DCC share
  the same GPU with the GeForce cards. The only difference in
  performance is that Quadro cards use special Professional features
  that are disabled in the GeForce cards. Until SoftQuadro was
  released, the only way to turn GeForce cards into Quadros was to
  physically solder several transistors on the boards thus forcing the
  card to send back a Device ID that corresponds to a Quadro card,
  thus enabling the drivers to recognize it as such. SoftQuadro simply
  patches the drivers and forces the Professional features of the card
  to be used. 

C. CONFIGURE YOUR CARD

 Read the NVIDIA-linux driver release notes on how to configure
your /etc/X11/XF86Config-4 (or Xorg config) file to support stereo
with the  option "Stereo".  This is added in the section "Device"
of the config file.  In mine, I have

Section "Device"
    Identifier "device1"
    VendorName "nVidia Corporation"
    BoardName "NVIDIA QuadroFx"
    Driver "nvidia"
    Option "Stereo" "1"

... etc. 

 I've not had experience with ATI's linux drivers but I've heard it 
isn't pleasant.  Hopefully ATI will get the Linux bug too and we will
see something competitive with Nvidia soon.


D. LCD SHUTTER GLASSES

If you have a supported card, you need a pair of LCD shutter
glasses and a Sync Doubler, see: 'http://www.stereo3d.com/control.htm'
I-glasses makes one of the nicest consumer level glasses, that
includes all the hardware needed for QuadBuffer stereo.  

They are at: 'http://www.i-glassesstore.com/'

The EXtreme 3D Game Glasses http://www.3dmedialabs.com/3dml-002.html
is pretty nice. $69US for the wired, $89US wireless. Wireless is nice
if you have a pair and you want a friend to look over your shoulder,
but beware, the batteries are not cheap at about $3 for a few weeks of
heavy use.  I like both; so get the wireless set first (includes a IR
emitter), and get the wired glasses separately.  They both plug into
the same sync-doubler VGA-pass through dongle and while you're wearing
the wired glasses, your fiends can be looking over your shoulder with
the wireless.  When the batteries on your wireless give out, you have
the wired glasses to fall back on.


E. SUPPORTED CRTs. 


  LCDs:

If you have an LCD monitor, I'm sorry to say there is no hope doing
stereo without going to some sort of polarizer systems or using other
methods (See ANAGLYPH AND OTHER OPTIONS).  However, there are some
interesting new methods for viewing stereo on LCD monitors, see:
'http://www.soft.lt/hardware/lcd/' for good concept.  For now, that is
kind of a home-brew way of stereo on LCDs.  There are some LCD
monitors available know as AUTOSTEREO LCDs (or autostereo displays)
that render 3D stereo graphics without glasses. I've seen and used
them and they are way cool but not cheap ($1500+).  I will say one
thing from experience with these monitors; eye separation from
software can either make the effect real or break your vision!.  Most
autostereo LCDs will still need a sync doubler just like LCD shutter
glasses, so you should get one of those anyway.  On the upside of
autostereo, Trigger will support it!


  CRTs:

For real 'Quadbuffer' stereo, the CRT monitor is a key piece of
equipment.  Everything from phosphor decay rates to refresh rates will
make a difference.  You will probably want a newer type digital
monitor, one that supports EDID (Extended Display Identification Data,
or sometime called DDC) signaling between the video card and the
monitor.  It's through this signaling that the monitor tells the
graphics card what resolutions and refresh rates it supports.  An old
analog VGA monitor that support 800x600 just is not going to work
well, but then, why would you be using a Quadro card to run Trigger in
stereo on a monitor like that?

  You really need CRT that will support a high vertical refresh rate
(75 - 85Hz or better).  The speed between toggling from a left-eye
view to a right-eye view is determined by the vertical sync rate.  A
monitor that supports 1280x1024 @ 60Hz will have a noticeable flicker
give about 30 Frames per Second (FPS) per Eye.  Many newer digital
monitors will do 640x480 @ 85Hz (about 42FPS/Eye) and has noticeably
less flicker.  On my monitor (An el-cheapo Cybervision Dx/86 17")
there is support for a 768x586 @ 100Hz which is very easy on the eyes.
On many of the SGI monitors I've used, most will support 1024x768@120
and some better than that.

I've found a really nice simple utility in the 'StereoGL' project on
SourceForge called 'vmode' in the /utils directory that will display
all of the modes a monitor will support.  It's a very hand utility to 
have when your working with stereo. Perhaps some soul will write
much needed 'setvmode' the set the display mode.
See: 'http://stereogl.sourceforge.net/'


F. ANAGLYPH AND OTHER OPTIONS

Stereo anaglyph viewing is where the left and right eye views are
color filtered and by wearing a pair of special colored glasses, the
corresponding views appear in three dimensions.  Here is a website
that will send you a pair of these glasses for FREE!
'http://www.rainbowsymphony.com/freestuff.html'

Another option that is actually not really stereo, but does give a
pretty good 3D effect is the so called Fresnel 'illusion' Box.  This
is basically a large Fresnel lens placed about 7 to 9" from the
monitor.  You can get these a many book stores for just a few dollars.
How they work is they make your eyes focus further into the screen,
which your brain interprets as depth, and you get that 3D effect.  It
does work from personal experience.  And it has the advantage that you
don't need special graphics hardware and works with all games. A more
detailed discussion of that is at:
'http://www.tacticalneuronics.com/content/Fresnel.asp'


G. TIPS.

You can toggle between the modes using X-Windows <CTRL><ALT><KEYPAD +> 
and <CTRL><ALT><KEYPAD -> or configure a default resolution in you 
/etc/X11/XF86Config-4 file.  

If you have a TV-tuner card running in the background, I've found
that sometimes that can interfere with the LCD shutter glasses sync.
You may need to remove the bttv device module to get the glasses to 
sync. As root, try: 'rmmod bttv' 
 
H. TRIGGER STEREO SUPPORT.

Stereo is configured with the trigger.config file under the
video settings.   

stereo="none"
    or "quadbuffer"
    or "red-blue"
    or "red-green"
    or "red-cyan"
    or "yellow-blue"
    - Enables stereo and selects a mode.

stereoeyeseperation="0.07"
    - Changes the images based on how far apart your eyes are.
      The value is the distance between your eyes in meters.
      Decrease this value if you feel uncomfortable or for
      children, but avoid increasing it too much.

stereoswapeyes="no" or "yes"
    - Set this to "yes" if the image appears to be swapped.


I. WARNING

  If you tend to get motion sickness, you should really not use the
stereo mode on 'Trigger'.  The so-called 'vomit' factor of this game
(based on MY psuedo-psycological scale of smooth and yet extreme
motions shifts with in the human perception :-) is close to a 10!
Ie., visually it's a roller coster!  Seriously though,  if you have
any problems like Epilepsy, you should not use this program option.

J. PERFORMANCE NOTES

Out of the five stereo modes available, quadbuffer (with appropriate
hardware) is likely to be the most efficient, and will definitely
give the best visual effect. However, many people will probably only
be able to use the anaglyph stereo modes (red-blue, etc).

It's worth noting that red-cyan and yellow-blue modes will have
the best performance. If you have red-blue or red-green glasses,
I suggest trying the red-cyan display mode first, since you will
get better framerates in the game. If this produces "ghosting"
effects, then try the red-blue or red-green modes as appropriate.

Many thanks to Chuck for a great Trigger add-on!

- Jaz ]

And many thanks to you Jaz, Richard (and the other contributors) 
for such a cool GPL'ed GAME!!! - Chuck ] 

