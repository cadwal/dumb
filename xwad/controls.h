#ifndef XWAD_CONTROLS_H
#define XWAD_CONTROLS_H

extern Display *dpy;
extern Window root;
extern int screen;

struct AppInstance;

/*extern GC ctlgc;
extern XFontStruct *ctlfont,*ctlfont1,*ctlfont2;
void setctlfont(XFontStruct *f);*/

/* disabled because of a GCC 2.7.2.1 bug with -pg */
#if 0 && defined(__GNUC__)&&defined(__i386__)
/* since many CTLACTIONs are just wrappers for one line of code,
   or a call to some other function, this saves a fair bit of space */
#define CTLATTR __attribute__((regparm (2)))
#else
#define CTLATTR
#endif

#define AppInst struct AppInstance

#define CTLACTION_(name) void name(AppInst *inst)
#define CTLPRED_(name) int name(AppInst *inst)
#define CTLINPUT_(name) void name(const char *buf,AppInst *inst) 
#define CTLOUTPUT_(name) void name(char *buf,const AppInst *inst)

#define CTLACTION_D(name) CTLACTION_(name) CTLATTR
#define CTLPRED_D(name) CTLPRED_(name) CTLATTR
#define CTLINPUT_D(name) CTLINPUT_(name) CTLATTR 
#define CTLOUTPUT_D(name) CTLOUTPUT_(name) CTLATTR
#define CTLACTION(name) CTLACTION_(name) CTLATTR; CTLACTION_(name) 
#define CTLPRED(name) CTLPRED_(name) CTLATTR; CTLPRED_(name)
#define CTLINPUT(name) CTLINPUT_(name) CTLATTR; CTLINPUT_(name) 
#define CTLOUTPUT(name) CTLOUTPUT_(name) CTLATTR; CTLOUTPUT_(name)

typedef CTLACTION_D((*CtlAction));
typedef CTLPRED_D((*CtlPred)); 
typedef CTLINPUT_D((*CtlInput));
typedef CTLOUTPUT_D((*CtlOutput));

#define CTLBUFLEN 12
#define CTLBUFMAX 8

typedef struct {
   char *name;
   unsigned int f;
   CtlAction action;
   CtlPred pred;
   CtlInput input;
   CtlOutput output;
} Control;

#define CTLF_BUTTONLIKE 0x00000001
#define CTLF_CANPRESS   0x00000002
#define CTLF_DANGER     0x00000004
#define CTLF_HASLITE    0x00000008
#define CTLF_HASBUF     0x00000010
#define CTLF_USEFONT2   0x00000020

#define IBUTTON(name,action,f) {name,CTLF_BUTTONLIKE|CTLF_CANPRESS|f,action,NULL,NULL,NULL}
#define LBUTTON(name,action,pred,f) {name,CTLF_BUTTONLIKE|CTLF_CANPRESS|CTLF_HASLITE|f,action,pred,NULL,NULL}
#define LABELCTL(name,f) {name,CTLF_BUTTONLIKE|f,NULL,NULL,NULL,NULL}
#define NULLCTL {NULL,0,NULL,NULL,NULL,NULL}
#define OUTCTL(out,f) {NULL,CTLF_HASBUF|f,NULL,NULL,NULL,out}
#define INPUTCTL(in,out,f) {NULL,CTLF_HASBUF|f,NULL,NULL,in,out}
#define INPUTCTLB(in,out,action,f) {NULL,CTLF_HASBUF|CTLF_CANPRESS|f,action,NULL,in,out}

typedef struct {
   char buf[CTLBUFLEN];
   int pressed:1;
   int editing:1;
   int disabled:1;
} ControlInstance,CtlInstance;

typedef struct {
   int cols,rows;
   const Control *ctls;
} ControlSet;

typedef struct {
   Window w;
   const ControlSet *cset;
   ControlInstance *inst;
} CSetInstance;

#define cseti_enable(cs,i,e) ((cs)->inst[i].disabled=((e)?0:1))

void expose_cseti(CSetInstance *cs,XExposeEvent *ev,AppInst *i);
void keyev_cseti(CSetInstance *cs,XKeyEvent *ev,AppInst *i);
void butev_cseti(CSetInstance *cs,XButtonEvent *ev,AppInst *i);
void motev_cseti(CSetInstance *cs,XMotionEvent *ev,AppInst *i);
void crossev_cseti(CSetInstance *cs,XCrossingEvent *ev,AppInst *i);

void rdlights_cseti(CSetInstance *csi,AppInst *inst);
void rdoutp_cseti(CSetInstance *csi,AppInst *inst);
void rdable_cseti(CSetInstance *csi,AppInst *inst);

void get_cset_size(const ControlSet *cset,int *width,int *height); 
#define get_cseti_size(c,w,h) get_cset_size((c)->cset,w,h)

void init_controls(Display *d,XFontStruct *f1,XFontStruct *f2);
void reset_controls(void);

void init_cseti(CSetInstance *csi,
		AppInst *inst,Window parent,const ControlSet *cset,int num);
void free_cseti(CSetInstance *csi);

#endif
