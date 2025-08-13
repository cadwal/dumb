#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>

#include "xwad.h"

#define RDMAPCTLS rdoutp_cseti(&inst->mapctls,inst)
#define RDMODECTLS rdoutp_cseti(&inst->modectls,inst)
#define RDRAW XClearArea(dpy,inst->map,0,0,0,0,True)


/* general control */

/*static CTLACTION(see_patches) {char buf[10];tchoose_patch(inst,buf);};*/
static CTLACTION(see_sprites) {char buf[10];tchoose_sprite(inst,buf);};

static CTLACTION(quit) {inst->want_quit=1;};
static CTLACTION(load) {load_instance(inst,inst->loadname);};
static CTLACTION(save) {save_level(inst);};

static CTLACTION(prev) {
   int i=inst->curselect;
   i--;
   if(i<0) i=maxsel(inst)-1;
   new_selection(i,inst,0);
};

static CTLACTION(next) {
   int i=inst->curselect,n=maxsel(inst);
   i++;
   if(n==0) i=-1;
   else if(i>=n) i=0;
   new_selection(i,inst,0);
};

static CTLACTION(del_thing) {
   int i=0,sel=inst->curselect;
   int survivor=-1;
   if(sel>=0) inst->enttbl[sel]|=ENT_SELECTED;
   while(i<inst->nthings) {
      if(inst->enttbl[i]&ENT_SELECTED) {
	 memmove(inst->thing+i,
		 inst->thing+i+1,
		 (--inst->nthings)-i);
	 inst->enttbl[i]^=ENT_SELECTED;
      }
      else if(survivor==-1) survivor=i++;
      else i++;
   };
   if(sel>=inst->nthings) sel=survivor;
   inst->curselect=sel+1;
   new_selection(sel,inst,0);
};
static CTLACTION(del_line) {
   int i=0,sel=inst->curselect;
   int survivor=-1;
   if(sel>=0) inst->enttbl[sel]|=ENT_SELECTED;
   while(i<inst->nlines) {
      if(inst->enttbl[i]&ENT_SELECTED) {
	 memmove(inst->line+i,
		 inst->line+i+1,
		 ((--inst->nlines)-i)*sizeof(LineData));
	 inst->enttbl[i]^=ENT_SELECTED;
      }
      else if(survivor==-1) survivor=i++;
      else i++;
   };
   if(sel>=inst->nlines) sel=survivor;
   inst->curselect=sel+1;
   new_selection(sel,inst,0);
};

static CTLACTION(line_split) {split_sel_lines(inst);RDRAW;};
static CTLACTION(mksector) {make_sector_from_sel_lines(inst);};
static CTLACTION(connect_selected_vertices) {connect_sel_vers(inst,0);};
static CTLACTION(connect_selected_vertices_ccw) {connect_sel_vers(inst,1);};

static CTLACTION(mkcorridor) {
   int s1=inst->curselect,s2;
   for(s2=0;s2<inst->nlines;s2++) 
      if(s1!=s2&&(inst->enttbl[s2]&ENT_SELECTED)) break;
   if(s2>=inst->nlines||s1<0) 
      qmessage(inst,"You must select exactly two lines.");
   else {
      int cs=make_corridor_between(inst,s1,s2);
      if(cs>=0) {
	 enter_mode(inst,SectMode);
	 new_selection(cs,inst,0);
      };
   };
};

#define MS(n) static CTLACTION(mkstairs##n) {\
  if(inst->curselect>=0) make_stairs(inst,inst->curselect,n);RDRAW;};
MS(8);
MS(16);
#undef MS

#define INST_STR_INP_FUNX(field,rd) \
static CTLINPUT(inp_##field) {strcpy(inst->field,buf);rd;}; \
static CTLOUTPUT(out_##field) {strcpy(buf,inst->field);}; 

#define INST_NUM_INP_FUNX(field,rd) \
static CTLINPUT(inp_##field) {inst->field=atoi(buf);rd;}; \
static CTLOUTPUT(out_##field) {sprintf(buf,"%d",(int)(inst->field));}; 

#define INST_FUNX(field,rd) \
static CTLPRED(is_##field) {return inst->field;}; \
static CTLACTION(tog_##field) {inst->field=!inst->field;rd;}; 

INST_FUNX(bigmap,update_intgeo(inst));
INST_FUNX(showgrid,RDRAW);

INST_STR_INP_FUNX(mapname,);
INST_STR_INP_FUNX(loadname,load_instance(inst,inst->loadname));

INST_NUM_INP_FUNX(gridsize,RDRAW);
/*INST_NUM_INP_FUNX(scale,RDRAW;RDMAPCTLS);*/

#define MODEFUNX(x,y) \
static CTLACTION(go##x##mode) {enter_mode(inst,y##Mode);}; \
static CTLPRED(is##x##mode) {return inst->mode==y##Mode;}

MODEFUNX(v,Ver);
MODEFUNX(l,Line);
MODEFUNX(s,Sect);
MODEFUNX(t,Thing);


#define FORSEL(typ) for(i=0;i<inst->n##typ##s;i++) \
if((inst->enttbl[i]&ENT_SELECTED)&&inst->curselect!=i)

/* vermode controls */

#define VALID (inst->curselect>=0&&inst->curselect<inst->nvers)
#define CUR (inst->ver[inst->curselect])
#define SEL(op) {int i; FORSEL(ver) inst->ver[i].op;} 

#define FUNX(x) \
static CTLINPUT(ver_inp_##x) \
{if(VALID) CUR.x=atoi(buf); SEL(x=atoi(buf)); RDRAW;} \
static CTLOUTPUT(ver_out_##x) \
{if(VALID) sprintf(buf,"%d",(int)(CUR.x)); else *buf=0;}

FUNX(x);
FUNX(y);

#undef VALID
#undef CUR
#undef SEL
#undef FUNX


/* sectmode controls */

#define VALID (inst->curselect>=0&&inst->curselect<inst->nsects)
#define CUR (inst->sect[inst->curselect])
#define SEL(op) {int i; FORSEL(sect) inst->sect[i].op;} 
#define SELSTRCPY(t,s) {int i; FORSEL(sect) strncpy(inst->sect[i].t,s,8);} 

#define FUNX(x) \
static CTLINPUT(sect_inp_##x) \
{if(VALID) CUR.x=atoi(buf); SEL(x=atoi(buf));} \
static CTLOUTPUT(sect_out_##x) \
{if(VALID) sprintf(buf,"%d",(int)(CUR.x)); else *buf=0;}

#define FUNXT(x) \
static CTLACTION(sect_act_##x) {if(VALID) \
tchoose_flat(inst,CUR.x);} \
static CTLINPUT(sect_inp_##x) \
{if(VALID) strncpy(CUR.x,buf,8); SELSTRCPY(x,buf); } \
static CTLOUTPUT(sect_out_##x) \
{memset(buf,0,CTLBUFLEN); if(VALID) strncpy(buf,CUR.x,8);}

FUNX(floor);
FUNX(ceiling);
FUNX(light);
FUNX(type);
FUNX(tag);

FUNXT(ftexture);
FUNXT(ctexture);

#undef VALID
#undef CUR
#undef SEL
#undef SELSTRCPY
#undef FUNX
#undef FUNXT


/* thingmode controls */

#define VALID (inst->curselect>=0&&inst->curselect<inst->nthings)
#define CUR (inst->thing[inst->curselect])
#define CURF (CUR.flags)
#define SEL(op) {int i; FORSEL(thing) inst->thing[i].op;} 

#define FUNX(x,rd) \
static CTLINPUT(th_inp_##x) \
{if(VALID) CUR.x=atoi(buf); SEL(x=atoi(buf)); rd;} \
static CTLOUTPUT(th_out_##x) \
{if(VALID) sprintf(buf,"%d",(int)(CUR.x)); else *buf=0;}

#define FLFUNX(flag) \
static CTLACTION(th_tog##flag) \
{if(VALID) {CURF^=THING_##flag;\
if(CURF&THING_##flag) {SEL(flags|=THING_##flag);} \
else SEL(flags&=~THING_##flag);};} \
static CTLPRED(th_is##flag) {return VALID&&(CURF&THING_##flag);}

FUNX(x,RDRAW);
FUNX(y,RDRAW);
FUNX(angle,RDRAW);
FUNX(type,);

FLFUNX(12);
FLFUNX(3);
FLFUNX(45);
FLFUNX(DEAF);
FLFUNX(MULTI);

#undef VALID
#undef CUR
#undef CURF
#undef SEL
#undef FUNX
#undef FLFUNX


/* linemode controls */

#define CUR (inst->line[inst->curselect])
#define CURF (CUR.flags)
#define LINEVALID (inst->curselect>=0&&inst->curselect<inst->nlines)
#define VALID LINEVALID
#define SEL(op) {int i; FORSEL(line) inst->line[i].op;} 

#define FLIP(l) {int v=l.ver1;l.ver1=l.ver2;l.ver2=v;}
static CTLACTION(line_vflip) {
   int i;
   if(VALID) FLIP(CUR);
   FORSEL(line) FLIP(inst->line[i]);
   RDMODECTLS;
   RDRAW;
};
#undef FLIP
#define FLIP(l) {int v=l.side[0];l.side[0]=l.side[1];l.side[1]=v;}
static CTLACTION(line_sflip) {
   int i;
   if(VALID) FLIP(CUR);
   FORSEL(line) FLIP(inst->line[i]);
   RDMODECTLS;
   /*RDRAW;*/
};
#undef FLIP

#define FLFUNX(flag) \
static CTLACTION(line_tog##flag) \
{if(VALID) {CURF^=LINE_##flag; \
if(CURF&LINE_##flag) {SEL(flags|=LINE_##flag);} \
else SEL(flags&=~LINE_##flag);};} \
static CTLPRED(line_is##flag) {return VALID&&(CURF&LINE_##flag);}

#define FUNX(x,rd) \
static CTLINPUT(line_inp_##x) \
{if(VALID) CUR.x=atoi(buf); SEL(x=atoi(buf)); rd;} \
static CTLOUTPUT(line_out_##x) \
{if(VALID) sprintf(buf,"%d",(int)(CUR.x)); else *buf=0;}

FUNX(ver1,RDRAW);
FUNX(ver2,RDRAW);
FUNX(tag,);
FUNX(type,);

FLFUNX(IMPASSIBLE);
FLFUNX(MIMPASSIBLE);
FLFUNX(UPUNPEG);
FLFUNX(LOUNPEG);
FLFUNX(SECRET);
FLFUNX(BLKSOUND);
FLFUNX(NOMAP);
FLFUNX(FORCEMAP);
FLFUNX(POSTER);

/* a special case, 2-sided should also make sure line has two sides */
static CTLACTION(line_togTWOSIDED) {
   int i;
   if(VALID) CURF^=LINE_TWOSIDED;
   if((CURF&LINE_TWOSIDED)&&CUR.side[1]==-1) {
      CUR.side[1]=inst->nsides++;
      RDMODECTLS;
   };
   FORSEL(line) {
      inst->line[i].flags^=LINE_TWOSIDED;
      if((inst->line[i].flags&LINE_TWOSIDED)&&inst->line[i].side[1]==-1)
	 inst->line[i].side[1]=inst->nsides++;
   };
} 
static CTLPRED(line_isTWOSIDED) {return VALID&&(CURF&LINE_TWOSIDED);}

#undef VALID
#undef CUR
#undef CURF
#undef SEL
#undef FUNX
#undef FLFUNX


/* linemode controls to do with sides */

#define SIDEID(sn) (inst->line[inst->curselect].side[sn])
#define SIDEVALID(sn) (LINEVALID&&(SIDEID(sn)>=0))
#define SIDE(sn) (inst->side[SIDEID(sn)])

#define SELSIDEID(sn) inst->line[i].side[sn]
#define SELSIDE(sn) inst->side[SELSIDEID(sn)]
#define FORSELSIDE(sn) FORSEL(line) if(SELSIDEID(sn)>=0)
#define SEL(sn,op) {int i; FORSELSIDE(sn) SELSIDE(sn).op;}
#define SELSTRCPY(sn,t,s) {int i; FORSELSIDE(sn) strncpy(SELSIDE(sn).t,s,8);} 

#define SIDE_IO_ID(sn) \
static CTLINPUT(side##sn##_inp_id) {if(LINEVALID) SIDEID(sn)=atoi(buf);\
if(SIDEID(sn)>=inst->nsides) inst->nsides=SIDEID(sn)+1;} \
static CTLOUTPUT(side##sn##_out_id) {if(LINEVALID) \
sprintf(buf,"%d",(int)(SIDEID(sn))); else *buf=0; };

#define SIDE_IO_SECT(sn) \
static CTLINPUT(side##sn##_inp_sect) {if(SIDEVALID(sn)) \
SIDE(sn).sector=atoi(buf); SEL(sn,sector=atoi(buf));} \
static CTLOUTPUT(side##sn##_out_sect) {if(SIDEVALID(sn)) \
sprintf(buf,"%d",(int)(SIDE(sn).sector)); else *buf=0; };

/* bit of a kludge to pack two offsets into one control */
static int atoi2(const char *s) {
   s=strchr(s,'/');
   if(s==NULL) return 0;
   s++;
   return atoi(s);
};

#define SIDE_IO_OFS(sn) \
static CTLINPUT(side##sn##_inp_ofs) { \
if(SIDEVALID(sn)) {SIDE(sn).xoffset=atoi(buf);SIDE(sn).yoffset=atoi2(buf);} \
SEL(sn,xoffset=atoi(buf));SEL(sn,yoffset=atoi2(buf));} \
static CTLOUTPUT(side##sn##_out_ofs) {if(SIDEVALID(sn)) \
sprintf(buf,"%d/%d",(int)(SIDE(sn).xoffset),(int)(SIDE(sn).yoffset)); else *buf=0; };

#define SIDE_IO_T(sn,t) \
static CTLACTION(side##sn##_act_##t) {if(SIDEVALID(sn)) \
tchoose_wall(inst,SIDE(sn).t);} \
static CTLINPUT(side##sn##_inp_##t) {if(SIDEVALID(sn)) \
strncpy(SIDE(sn).t,buf,8); SELSTRCPY(sn,t,buf);} \
static CTLOUTPUT(side##sn##_out_##t) {memset(buf,0,CTLBUFLEN); \
if(SIDEVALID(sn)) strncpy(buf,SIDE(sn).t,8);};

SIDE_IO_ID(0);
SIDE_IO_ID(1);

SIDE_IO_SECT(0);
SIDE_IO_SECT(1);

SIDE_IO_OFS(0);
SIDE_IO_OFS(1);

#define SIDET(t) SIDE_IO_T(0,t);SIDE_IO_T(1,t)

SIDET(texture);
SIDET(utexture);
SIDET(ltexture);


/* map controls */

static CTLACTION(zoomi) {
   if(inst->scale>-8) {
      inst->scale--;
      RDRAW;RDMAPCTLS;
   };
};
static CTLACTION(zoomo) {
   if(inst->scale<8) {
      inst->scale++;
      RDRAW;RDMAPCTLS;
   };
};

static CTLOUTPUT(out_scale) {
   if(inst->scale<0) sprintf(buf,"1:%d",1<<-inst->scale);
   else sprintf(buf,"%d:1",1<<inst->scale);
};

static CTLINPUT(selectinp) {
   new_selection(atoi(buf),inst,0);
};
static CTLOUTPUT(selectout) {
   if(inst->curselect<0) *buf=0;
   else sprintf(buf,"%d",inst->curselect);
};


/* control structures */

#define GENROWS 5
#define GENCOLS 3

static const Control gen_ctls[GENROWS*GENCOLS]={
IBUTTON("Quit",quit,CTLF_DANGER),
LBUTTON("BigMap",tog_bigmap,is_bigmap,0),
LBUTTON("VerMode",govmode,isvmode,0),

IBUTTON("Load:",load,0),
INPUTCTL(inp_loadname,out_loadname,CTLF_USEFONT2),
LBUTTON("LineMode",golmode,islmode,0),

IBUTTON("Save As:",save,0),
INPUTCTL(inp_mapname,out_mapname,CTLF_USEFONT2),
LBUTTON("SectMode",gosmode,issmode,0),

IBUTTON("Prev",prev,0),
IBUTTON("Next",next,0),
LBUTTON("ThingMode",gotmode,istmode,0),

LBUTTON("Grid:",tog_showgrid,is_showgrid,0),
INPUTCTL(inp_gridsize,out_gridsize,0),
IBUTTON("Sprites",see_sprites,0)
}; 

#define MAPROWS 1
#define MAPCOLS 4

static const Control map_ctls[MAPROWS*MAPCOLS]={
IBUTTON("Zoom In",zoomi,0),
IBUTTON("Zoom Out",zoomo,0),
LABELCTL("Scale:",0),
OUTCTL(out_scale,0)
}; 


/* mode control structures other than line */


#define INPAIR(pref,name,field,f) LABELCTL(name,0),\
INPUTCTL(pref##_inp_##field,pref##_out_##field,f)
#define INBPAIR(pref,name,field,f) LABELCTL(name,0),\
INPUTCTLB(pref##_inp_##field,pref##_out_##field,pref##_act_##field,f)

#define VERROWS 4
#define VERCOLS 2

static const Control vermode_ctls[VERROWS*VERCOLS]={
LABELCTL("Vertex #",0),
INPUTCTL(selectinp,selectout,0),
INPAIR(ver,"X-coord",x,0),
INPAIR(ver,"Y-coord",y,0),
IBUTTON("Connect",connect_selected_vertices,0),
IBUTTON("CCW",connect_selected_vertices_ccw,0)
};

#define SECTROWS 9
#define SECTCOLS 2

static const Control sectmode_ctls[SECTROWS*SECTCOLS]={
LABELCTL("Sect #",0),
INPUTCTL(selectinp,selectout,0),
INPAIR(sect,"FloorLvl",floor,0),
INPAIR(sect,"CeilingLvl",ceiling,0),
INBPAIR(sect,"Floor",ftexture,CTLF_USEFONT2),
INBPAIR(sect,"Ceiling",ctexture,CTLF_USEFONT2),
INPAIR(sect,"Light",light,0),
INPAIR(sect,"Type",type,0),
INPAIR(sect,"Tag",tag,0),
IBUTTON("Stairs",mkstairs8,0),
IBUTTON("BigStairs",mkstairs16,0)
}; 

#define TFLAG_BUTTON(name,flag) LBUTTON(name,th_tog##flag,th_is##flag,0)

#define THROWS 8
#define THCOLS 2

static const Control thingmode_ctls[THROWS*THCOLS]={
LABELCTL("Thing #",0),
INPUTCTL(selectinp,selectout,0),
INPAIR(th,"X-coord",x,0),
INPAIR(th,"Y-coord",y,0),
TFLAG_BUTTON("Easy",12),
IBUTTON("Delete",del_thing,CTLF_DANGER),
TFLAG_BUTTON("Medium",3),
TFLAG_BUTTON("Multi",MULTI),
TFLAG_BUTTON("Hard",45),
TFLAG_BUTTON("Deaf",DEAF),
INPAIR(th,"Angle",angle,0),
INPAIR(th,"Type",type,0)
}; 


/* linemode control structures (more complicated than the others) */

#define LFLAG_BUTTON(name,flag) LBUTTON(name,line_tog##flag,line_is##flag,0)

#define LINE_ICTL(x) INPUTCTL(line_inp_##x,line_out_##x,0)

#define SIDEINPC(n,f,l) INPUTCTL(side##n##_inp_##f,side##n##_out_##f,l)
#define SIDEINPB(n,f,l) INPUTCTLB(\
                 side##n##_inp_##f,side##n##_out_##f,side##n##_act_##f,l)
#define SIDE_BUTTONS(name,f,l) LABELCTL(name,0),\
                               SIDEINPC(0,f,l),SIDEINPC(1,f,l)
#define SIDE_BUTTONSB(name,f,l) LABELCTL(name,0),\
                                SIDEINPB(0,f,l),SIDEINPB(1,f,l)

#define LINEROWS 15
#define LINECOLS 3

static const Control linemode_ctls[LINEROWS*LINECOLS]={
LABELCTL("Line #",0),
INPUTCTL(selectinp,selectout,0),
IBUTTON("Delete",del_line,CTLF_DANGER),

INPAIR(line,"Tag",tag,0),
IBUTTON("VerFlip",line_vflip,0),

INPAIR(line,"Type",type,0),
IBUTTON("SideFlip",line_sflip,0),

LABELCTL("Vertex #",0),
LINE_ICTL(ver1),
LINE_ICTL(ver2),

LFLAG_BUTTON("Impassible",IMPASSIBLE),
LFLAG_BUTTON("TwoSided",TWOSIDED),
LFLAG_BUTTON("M-Impass",MIMPASSIBLE),

LFLAG_BUTTON("UnpUpper",UPUNPEG),
LFLAG_BUTTON("UnpLower",LOUNPEG),
LFLAG_BUTTON("Secret",SECRET),

LFLAG_BUTTON("BlockSnd",BLKSOUND),
LFLAG_BUTTON("NoMap",NOMAP),
LFLAG_BUTTON("ForceMap",FORCEMAP),

LFLAG_BUTTON("Poster",POSTER),
NULLCTL,
NULLCTL,

IBUTTON("Split",line_split,0),
IBUTTON("MkSector",mksector,0),
IBUTTON("Corridor",mkcorridor,0),

SIDE_BUTTONS("Side #",id,0),
SIDE_BUTTONS("Sector #",sect,0),
SIDE_BUTTONS("Offset",ofs,0),
SIDE_BUTTONSB("Texture",texture,CTLF_USEFONT2),
SIDE_BUTTONSB("U-Texture",utexture,CTLF_USEFONT2),
SIDE_BUTTONSB("L-Texture",ltexture,CTLF_USEFONT2)
}; 


/* control sets (only these need be non-static) */

const ControlSet gen_cset[1]={{GENCOLS,GENROWS,gen_ctls}};
const ControlSet map_cset[1]={{MAPCOLS,MAPROWS,map_ctls}};
const ControlSet mode_csets[NumModes]={
   {VERCOLS,VERROWS,vermode_ctls},
   {LINECOLS,LINEROWS,linemode_ctls},
   {SECTCOLS,SECTROWS,sectmode_ctls},
   {THCOLS,THROWS,thingmode_ctls}
};





