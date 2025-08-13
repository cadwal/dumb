/* ------------------------------------------------------------------------
 *       ggi_video.c
 * ------------------------------------------------------------------------
 */

#include <config.h>

#include <ggi/maintainers.h>
#define MAINTAINER      "Andrew Apted <ajapted@ggi-project.org>"
#define REVISION        "Revision: 1.5"

/* Undefine this to get the keys back to what they were before I
 * messed with them.  - Kalle */
#define KALLE_KEYS 1

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>   /* for lib/log.h */

#include "libdumbutil/safem.h"
#include "libdumbutil/log.h"
#include "input.h"
#include "video.h"

#include <ggi/libggi.h>

#define KK_ESC    '\033'
#define KK_TAB    '\011'
#define KK_SPACE  '\040'

#define STATE_SHIFT        0x000001
#define STATE_ALT          0x000002
#define STATE_CTRL         0x000004
#define STATE_STRAFE       0x000008  /* mouse button */

#define STATE_FORWARD      0x000010
#define STATE_BACKWARD     0x000020
#define STATE_LEFT         0x000040
#define STATE_RIGHT        0x000080

#define STATE_LOOKUP       0x000100
#define STATE_LOOKDOWN     0x000200
#define STATE_LOOKCENTRE   0x000400  /* not implemented */

#define STATE_UP           0x001000
#define STATE_DOWN         0x002000
#define STATE_FIRE         0x004000

#define STATE_ROTLEFT      0x010000
#define STATE_ROTRIGHT     0x020000
#define STATE_SIDELEFT     0x040000
#define STATE_SIDERIGHT    0x080000

#define RUN_SPEED  (UNIT_SPEED << 1)

#define MOUSE_X_SENSITIVITY  24;
#define MOUSE_Y_SENSITIVITY  16;


ConfItem input_conf[]={
   /* no input configs yet... */
   /* one idea: --input-sensitivity 24 */
   { NULL }
};

ConfItem video_conf[]={
   /* no video configs yet... */
   /* one idea: --video-target display-X */
   { NULL }
};


struct ggi_screen_info
{
        ggi_visual_t vis;

        int width, height, bpp;

        void *pagev; int pagelen;

        uint32 kb_state;
};

static struct ggi_screen_info GSI;


/*****  GGI Graphics  *****/


void video_preinit(void)
{
        ggiInit();
        GSI.vis = ggiOpen(NULL);

        if (GSI.vis == NULL) {
                logfatal('V',"GGI: ggiOpen failed.");
        }

        GSI.pagev = NULL;
}

void init_video(int *width, int *height, int *bpp, int *real_width)
{
        int err;

        if (GSI.pagev != NULL) {
                safe_free(GSI.pagev);
                GSI.pagev = NULL;
        }

        switch(*bpp)
	{
                case 1:
                        err = ggiSetGraphMode(GSI.vis, *width,*height,
                                *width,*height, GT_8BIT);
                        break;

                case 2:
                        err = ggiSetGraphMode(GSI.vis, *width,*height,
                                *width,*height, GT_16BIT);
                        break;

                case 3:
                        err = ggiSetGraphMode(GSI.vis, *width,*height,
                                *width,*height, GT_24BIT);
                        break;

                case 4:
                        err = ggiSetGraphMode(GSI.vis, *width,*height,
                                *width,*height, GT_32BIT);
                        break;

                default:
                        logfatal('V', "Bad BPP (%d) in init_video", *bpp);
        }

        if (err) {
                logfatal('V', "GGI: SetGraphMode failed.");
        }

        GSI.width  = *width  = ggiGetInfo(GSI.vis)->fb.width;
        GSI.height = *height = ggiGetInfo(GSI.vis)->fb.height;
        GSI.bpp    = *bpp; 

	*real_width = GSI.width;

        GSI.pagelen = GSI.width * GSI.height * GSI.bpp;

        logprintf(LOG_INFO, 'V', "init_video in %s mode pagelen=%d",
                        "copying", GSI.pagelen);

        GSI.pagev = safe_malloc(GSI.pagelen);
	
	ggiSetInfoFlags(GSI.vis, GGIFLAG_ASYNC);
}

void reset_video(void)
{
        if (GSI.pagev != NULL) {
                safe_free(GSI.pagev);
                GSI.pagev=NULL;
        }

        ggiExit();
}

void video_setpal(unsigned char index, unsigned char red,
                  unsigned char green, unsigned char blue)
{
        ggi_color gcol;

        #define SHLPAL  8

        gcol.r = red   << SHLPAL;
        gcol.g = green << SHLPAL;
        gcol.b = blue  << SHLPAL;

        ggiSetPaletteVec(GSI.vis, index, 1, &gcol);

        #undef SHLPAL
}

void *video_newframe(void)
{
        return GSI.pagev;
}

void video_updateframe(void *v)
{
        ggiPutBox(GSI.vis, 0,0, GSI.width,GSI.height, v);
        ggiFlush(GSI.vis);   /* render NOW ! */
}

void video_winstuff(const char *desc, int xdim, int ydim)
{
        /* nothing to do */
}


/*****  GGI Input  *****/


#define GGI_EVENTS  (emKey | emPointer)

#define MODSTATE(flag, state)  if (state) GSI.kb_state |= (flag);  \
        else GSI.kb_state &= ~(flag);


static void handle_ggi_key_event(ggi_event *ev, PlayerInput *in)
{
        int state;
        int sym;

        if (ev->any.type == evKeyPress) {
                state=1;
        } else if (ev->any.type == evKeyRelease) {
                state=0;
        } else {
                return;
        }

        sym = U(ev->key.sym);

        if ((ev->key.sym == K_VOID) || (sym == K_VOID)) {
                return;
        }
        
        if ((KTYP(sym) == KT_LATIN) || (KTYP(sym) == KT_LETTER) ||
	    (KTYP(sym) == KT_META)) {

                switch(KVAL(sym))
                {
                        /* once only actions */

                        case KK_ESC: in->quit=state; break;

                        case '0': in->select[0]=state; break;
                        case '1': in->select[1]=state; break;
                        case '2': in->select[2]=state; break;
                        case '3': in->select[3]=state; break;
                        case '4': in->select[4]=state; break;
                        case '5': in->select[5]=state; break;
                        case '6': in->select[6]=state; break;
                        case '7': in->select[7]=state; break;
                        case '8': in->select[8]=state; break;
                        case '9': in->select[9]=state; break;

#ifdef KALLE_KEYS
			case 'q': case 'Q'-64:
			case 'Q': in->w_sel = -state; break;
				
			case 'w': case 'W'-64:
			case 'W': in->w_sel = state; break;
#else  /* !KALLE_KEYS */
                        case 'v': case 'V': case '\026':
                        case KK_TAB: in->w_sel=state; break;
#endif /* !KALLE_KEYS */

                        /* continuous actions */

#ifdef KALLE_KEYS
			case 'h': case 'H'-64:
			case 'H': MODSTATE(STATE_LEFT, state); break;

			case 'l': case 'L'-64:
			case 'L': MODSTATE(STATE_RIGHT, state); break;

			case 'k': case 'K'-64:
			case 'K': MODSTATE(STATE_FORWARD, state); break;

			case 'j': case 'J'-64:
			case 'J': MODSTATE(STATE_BACKWARD, state); break;

			case 's': case 'S'-64:
			case 'S': MODSTATE(STATE_FIRE, state); break;
#else  /* !KALLE_KEYS */
                        case 'h': case '\010':
                        case 'H': MODSTATE(STATE_LEFT, state); break;

                        case 'k': case '\013':
                        case 'K': MODSTATE(STATE_RIGHT, state); break;

                        case 'u': case '\025':
                        case 'U': MODSTATE(STATE_FORWARD, state); break;

                        case 'j': case '\012':
                        case 'J': MODSTATE(STATE_BACKWARD, state); break;

                        case 'f': case '\006':
                        case 'F': MODSTATE(STATE_FIRE, state); break;
#endif /* !KALLE_KEYS */

                        case ',':
                        case '<': MODSTATE(STATE_SIDELEFT, state); break;

                        case '.':
                        case '>': MODSTATE(STATE_SIDERIGHT, state); break;

#ifdef KALLE_KEYS
			case 'a': case 'A'-64:
			case 'A': MODSTATE(STATE_UP, state); break;

			case 'z': case 'Z'-64:
			case 'Z': MODSTATE(STATE_DOWN, state); break;

			case 'd': case 'D'-64:
			case 'D': MODSTATE(STATE_LOOKUP, state); break;

			case 'c': case 'C'-64:
			case 'C': MODSTATE(STATE_LOOKDOWN, state); break;
#else  /* !KALLE_KEYS */
                        case 'a': case '\001':
                        case 'A': MODSTATE(STATE_LOOKUP, state); break;

                        case 'z': case '\032':
                        case 'Z': MODSTATE(STATE_LOOKDOWN, state); break;

                        case 'd': case KK_SPACE:
                        case 'D': MODSTATE(STATE_UP, state); break;

                        case 'c':
                        case 'C': MODSTATE(STATE_DOWN, state); break;
#endif /* !KALLE_KEYS */

                        default: printf("Unused key: Latin "
                                "sym=%04x (%c)\n", sym, KVAL(sym));
                }
        } else {
                switch(sym)
                {
                        /* once only actions */

                        /* continuous actions */

#ifdef KALLE_KEYS
			case K_UP: case K_ASC8:
			case K_P8: MODSTATE(STATE_FORWARD, state); break;

			case K_DOWN: case K_ASC2:
			case K_P2: MODSTATE(STATE_BACKWARD, state); break;

			case K_LEFT: case K_DECRCONSOLE: case K_ASC4:
			case K_P4: MODSTATE(STATE_LEFT, state); break;

			case K_RIGHT: case K_INCRCONSOLE: case K_ASC6:
			case K_P6: MODSTATE(STATE_RIGHT, state); break;

#ifdef K_NORMAL_CTRL
			case K_NORMAL_CTRL:
#endif
#ifdef K_NORMAL_CTRLL
			case K_NORMAL_CTRLL: case K_NORMAL_CTRLR:
#endif
				MODSTATE(STATE_FIRE, state); break;

                        case K_PGUP: case K_SCROLLBACK: case K_ASC9:
                        case K_P9: MODSTATE(STATE_LOOKUP, state); break;

                        case K_PGDN: case K_SCROLLFORW: case K_ASC7:
			case K_P7: MODSTATE(STATE_LOOKDOWN, state); break;

			case K_ASC1:
                        case K_P1: MODSTATE(STATE_SIDELEFT, state); break;

			case K_ASC3:
                        case K_P3: MODSTATE(STATE_SIDERIGHT, state); break;

                        case K_INSERT: MODSTATE(STATE_UP, state); break;

                        case K_REMOVE: MODSTATE(STATE_DOWN, state); break;

#else  /* !KALLE_KEYS */
                        case K_UP:
                        case K_P5: MODSTATE(STATE_FORWARD, state); break;

                        case K_DOWN:
                        case K_P2: MODSTATE(STATE_BACKWARD, state); break;

                        case K_RIGHT:
                        case K_INCRCONSOLE:
                        case K_P6: MODSTATE(STATE_RIGHT, state); break;

                        case K_LEFT:
                        case K_DECRCONSOLE:
                        case K_P4: MODSTATE(STATE_LEFT, state); break;

                        case K_ENTER: MODSTATE(STATE_FIRE, state); break;

                        case K_PGUP:
                        case K_SCROLLBACK:
                        case K_P9: MODSTATE(STATE_LOOKUP, state); break;

                        case K_PGDN:
                        case K_SCROLLFORW:
                        case K_P3: MODSTATE(STATE_LOOKDOWN, state); break;

                        case K_INSERT:
                        case K_P7: MODSTATE(STATE_UP, state); break;

                        case K_REMOVE:
                        case K_P1: MODSTATE(STATE_DOWN, state); break;
#endif /* !KALLE_KEYS */

                        /* modifiers */
#ifdef K_NORMAL_SHIFT
                        case K_NORMAL_SHIFT: MODSTATE(STATE_SHIFT, state);
                        break;
#endif
#ifdef K_NORMAL_ALT
                        case K_NORMAL_ALT: MODSTATE(STATE_ALT, state);
                        break;
#endif
#ifdef K_NORMAL_ALTGR
                        case K_NORMAL_ALTGR: MODSTATE(STATE_ALT, state);
                        break;
#endif
#ifndef KALLE_KEYS
#ifdef K_NORMAL_CTRL
                        case K_NORMAL_CTRL: MODSTATE(STATE_CTRL, state); 
                        break;
#endif
#endif /* !KALLE_KEYS */
                        default: printf("Unused key: Special "
                                        "sym=%04x\n", sym);
                }
        }
}

static void handle_ggi_ptr_event(ggi_event *ev, PlayerInput *in)
{
        int state;
        int speed = (GSI.kb_state & STATE_SHIFT) ? RUN_SPEED : UNIT_SPEED;

        if (ev->any.type == evPtrRelative) {

                int dx = -ev->pmove.x / 2;
                int dy = -ev->pmove.y;

                dx = (dx * speed) / MOUSE_X_SENSITIVITY;
                dy = (dy * speed) / MOUSE_Y_SENSITIVITY;

#ifdef KALLE_KEYS
                if (GSI.kb_state & (STATE_ALT | STATE_STRAFE)) {
#else
                if (GSI.kb_state & (STATE_CTRL | STATE_STRAFE)) {
#endif
                        in->sideways += dx;
                } else {
                        in->rotate += dx;
                }

                in->forward += dy;
                return;
        }

        if (ev->any.type == evPtrAbsolute) {

                /* this'll be fun... :) */

                int dx = (-ev->pmove.x + (GSI.width >>1)) * speed /
                                (GSI.width>>2);
                int dy = (-ev->pmove.y + (GSI.height>>1)) * speed /
                                (GSI.height>>2);

#ifdef KALLE_KEYS
                if (GSI.kb_state & (STATE_ALT | STATE_STRAFE)) {
#else
                if (GSI.kb_state & (STATE_CTRL | STATE_STRAFE)) {
#endif
                        in->sideways += dx;
                } else {
                        in->rotate += dx;
                }

                in->forward += dy;
                return;
        }

        if (ev->any.type == evPtrButtonPress) {
                state=1;
        } else if (ev->any.type == evPtrButtonRelease) {
                state=0;
        } else {
                return;
        }

        if (ev->pbutton.button & 1) {  /* primary button */
                MODSTATE(STATE_FIRE, state);
        }

        if (ev->pbutton.button & 2) {  /* secondary button */
                MODSTATE(STATE_FORWARD, state);
        }

        if (ev->pbutton.button & 4) {  /* tertiary button */
                MODSTATE(STATE_STRAFE, state);
        }
}

static void update_continuous_actions(PlayerInput *in)
{
        #define SPEEDOF(flag)  ((GSI.kb_state & (flag)) ? \
                ((GSI.kb_state & STATE_SHIFT) ? RUN_SPEED : UNIT_SPEED) : 0)

        in->forward  += SPEEDOF(STATE_FORWARD)  - SPEEDOF(STATE_BACKWARD);
        in->lookup   += SPEEDOF(STATE_LOOKUP)   - SPEEDOF(STATE_LOOKDOWN);
        in->sideways += SPEEDOF(STATE_SIDELEFT) - SPEEDOF(STATE_SIDERIGHT);
        in->rotate   += SPEEDOF(STATE_ROTLEFT)  - SPEEDOF(STATE_ROTRIGHT);

#ifdef KALLE_KEYS
        if (GSI.kb_state & (STATE_ALT | STATE_STRAFE)) {
#else
        if (GSI.kb_state & (STATE_CTRL | STATE_STRAFE)) {
#endif
                in->sideways += SPEEDOF(STATE_LEFT) - SPEEDOF(STATE_RIGHT);
        } else {
                in->rotate   += SPEEDOF(STATE_LEFT) - SPEEDOF(STATE_RIGHT);
        }

        in->jump += SPEEDOF(STATE_UP);   /* flying */

        if (GSI.kb_state & STATE_FIRE) {
                in->shoot |= 1;
        }

        #undef SPEEDOF
}

void get_input(PlayerInput *in)
{
        ggi_event ev;

        memset(in, 0, sizeof(PlayerInput));

        for (;;) {
                struct timeval tv = { 0, 0 };

                if (ggiEventPoll(GSI.vis, GGI_EVENTS, &tv) == 0) {
                        break;
                }

                ggiEventRead(GSI.vis, &ev, GGI_EVENTS);

                handle_ggi_key_event(&ev, in);
                handle_ggi_ptr_event(&ev, in);
        }

        update_continuous_actions(in);
}

void init_input(void)
{ 
        GSI.kb_state=0;
}

void reset_input(void)
{
        /* nothing to do */
}

// Local Variables:
// c-basic-offset: 8
// c-file-offsets: ((substatement-open . 0) (case-label . +))
// End:
