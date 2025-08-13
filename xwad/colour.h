#ifndef XWAD_COLOUR_H
#define XWAD_COLOUR_H

typedef enum {
   Background,
   Border,
   UnpressedCtl,
   PressedCtl,
   HighlightCtl,
   LowlightCtl,
   UnpressedText,
   PressedText,
   UnpressedDanger,
   PressedDanger,
   HighlightDanger,
   LowlightDanger,
   MapBg,
   MapFg,
   MapShowFg,
   ScaleFg,
   LitLight,
   UnlitLight,
   MapCurSelectFg,
   MapSelectFg,
   MapAnnotateFg,
   MapMessageFg,
   MapGridFg,
   ChooseBg,
   ChooseFg,
   ChooseCurBg,
   ChooseCurFg,
   MapTaggedFg,
   NumCtlColours
} CtlColour;

#define XCINIT(c,b) {b,(c>>8)&0xff00,c&0xff00,(c<<8)&0xff00,DoRed|DoGreen|DoBlue,0}

extern int controls_3d;
extern XColor ctl_colours[NumCtlColours];
extern XColor pal_colours[256];
extern Colormap ctl_cmap,pal_cmap;

#define CTLC(c) (ctl_colours[c].pixel)
#define PALC(c) (pal_colours[c].pixel)

void init_colour(void);
void reset_colour(void);

#endif
