#include <config.h>

#include <stdio.h>
#include <limits.h>
#include <ctype.h>

#include <aalib.h>

#include "libdumbutil/log.h"
#include "input.h"
#include "keymap.h"
#include "keyname.h"

#define SHIFT_KEYCODE (AA_UNKNOWN-1) /* should not be used by AA-lib */

ConfItem input_conf[]={
   CONFI("autorelease",NULL,0,"ticks to pass before key autorelease or 0", 1),
   CONFI("autorelease-shift",NULL,0,"ticks to pass before shift autorelease or 0", 1),
   {NULL}
};
#define cnf_autorelease (input_conf[0].intval)
#define cnf_autorelease_shift (input_conf[1].intval)

extern aa_context *aa_ctxt;
#define aa aa_ctxt

/* When this is 0, get_input() assumes that the current AA-lib
 * keyboard driver doesn't send events with AA_RELEASE and therefore
 * internally releases the previous key whenever another one is
 * pressed.  If get_input() gets AA_RELEASE, it sets this to 1.  */
static int supports_release;

/* The key that is currently held down and will be assumed to have
 * been released when supports_release is 0 and some other key is
 * pressed.  When supports_release is 1, this variable isn't read.  */
static keymap_keycode held_key;

/* How many ticks have passed since the key was pressed.  When this
 * exceeds cnf_autorelease, the key is assumed to have been released,
 * except if cnf_autorelease==0 or supports_release==1.  This variable
 * isn't used when held_key==KEYMAP_NULL_KEYCODE.  */
static int ticks_held;

/* Like ticks_held but for the Shift key.  This key is even trickier
 * than the others, as presses aren't reported separately.  Because
 * AA-lib doesn't define a keycode for Shift, this is used even when
 * supports_release==1.  This variable also serves as a flag; it's -1
 * when Shift is assumed not to be held.  */
static int ticks_shift_held;
   
void
get_input(PlayerInput *in)
{
   int ev;
   while (ev=aa_getevent(aa, 0), ev!=AA_NONE) {
      int pressed;		/* 0=release, 1=press */
      if (ev & AA_RELEASE) {
	 ev &= ~AA_RELEASE;
	 pressed = 0;
      } else
	 pressed = 1;
      /* FIXME: Is it compatible to call isupper() with non-characters?  */
      if (isupper(ev)) {
	 /* shifted */
	 ev = tolower(ev);
	 if (ticks_shift_held == -1)
	    keymap_press_keycode(SHIFT_KEYCODE, 1);
	 ticks_shift_held = 0;
      } else if (islower(ev)) {
	 /* not shifted */
	 if (ticks_shift_held != -1)
	    keymap_press_keycode(SHIFT_KEYCODE, 0);
	 ticks_shift_held = -1;
      }
      if (pressed) {
	 if (!supports_release && held_key!=KEYMAP_NULL_KEYCODE) {
	    /* assume previous key has been released */
	    keymap_press_keycode(held_key, 0);
	 }
	 keymap_press_keycode(ev, 1);
	 held_key = ev;
	 ticks_held = 0;
      } else {
	 keymap_press_keycode(ev, 0);
	 supports_release = 1;
      }
   }
   if (!supports_release && cnf_autorelease 
       && held_key!=KEYMAP_NULL_KEYCODE
       && ticks_held++ >= cnf_autorelease) {
      keymap_press_keycode(held_key, 0);
      held_key = KEYMAP_NULL_KEYCODE;
   }
   if (cnf_autorelease_shift
       && ticks_shift_held != -1
       && ticks_shift_held++ >= cnf_autorelease) {
      keymap_press_keycode(SHIFT_KEYCODE, 0);
      ticks_shift_held = -1;
   }
   ctlkey_calc_tick();
   ctlkey_get_player_input(in);
}

void
init_input(void)
{
   if(aa==NULL)
      logfatal('I', "aalib_input must be initialised after aalib_video");
   aa_autoinitkbd(aa, AA_SENDRELEASE);
   supports_release = 0;
   held_key = KEYMAP_NULL_KEYCODE;
   ticks_shift_held = -1;
}

void
reset_input(void)
{
}

const char *
keymap_keycode_to_keyname(keymap_keycode keycode)
{
   static char buf[20];		/* "aalib_-2147483678" must fit */
   switch (keycode) {
      /* AA_RESIZE and AA_MOUSE should be handled earlier but if they
       * get here, at least we have names for them.  */
   case AA_RESIZE: return "Resize";
   case AA_MOUSE: return "Mouse";
   case AA_UP: return "Up";
   case AA_DOWN: return "Down";
   case AA_LEFT: return "Left";
   case AA_RIGHT: return "Right";
   case AA_BACKSPACE: return "BackSpace";
   case AA_ESC: return "Escape";
   case SHIFT_KEYCODE: return "Shift";

   default:
      if (keycode>=CHAR_MIN && keycode<=CHAR_MAX) {
	 return keyname_of_char((char) keycode);
      } else {
	 sprintf(buf, "aalib_%ld", (long int) keycode);
	 return buf;
      }
   } /* switch (keycode) */
}

void
keymap_free_keyname(const char *keyname)
{
   /* the string was allocated statically -- no need to free anything */
}

// Local Variables:
// c-basic-offset: 3
// End:
