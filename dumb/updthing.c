#include <stdarg.h>
#include <math.h>
#include <limits.h>
#include <stdio.h>

#include "lib/log.h"
#include "lib/timer.h"
#include "levdata.h"
#include "gettable.h"
#include "things.h"

#define SLIDE_BAILOUT 4
#define GRAVITY (FIXED_ONE)

#define TICKDIV (2*MSEC_PER_TICK)

fixed line_dist(const LevData *ld,int wall,fixed x,fixed y) {
   double cx=FIXED_TO_FLOAT(x),cy=FIXED_TO_FLOAT(y);
   double ax=FIXED_TO_FLOAT(ldvertexd(ld)[ldline(ld)[wall].ver1].x);
   double ay=FIXED_TO_FLOAT(ldvertexd(ld)[ldline(ld)[wall].ver1].y);
   double bx=FIXED_TO_FLOAT(ldvertexd(ld)[ldline(ld)[wall].ver2].x);
   double by=FIXED_TO_FLOAT(ldvertexd(ld)[ldline(ld)[wall].ver2].y);

   const double delta_x = bx-ax;
   const double delta_y = by-ay;
   double line_len, r, s, t;
   /* the length of the line in **2 */
   line_len = delta_x*delta_x+delta_y*delta_y;
   //logprintf(LOG_DEBUG,'O',"line_dist: ll=%f a=(%f,%f), b=(%f,%f)",line_len,ax,ay,bx,by);
   r = ((ay-cy)*(-delta_y)-(ax-cx)*delta_x)/line_len;
   /* If s becomes negative we are facing the line clockwise */
   s = ((ay-cy)*(delta_x)-(ax-cx)*delta_y)/sqrt( line_len );
   //logprintf(LOG_DEBUG,'O',"line_dist: s=%f r=%f",s,r);

   if(r<0.0) t=sqrt((cx-ax)*(cx-ax)+(cy-ay)*(cy-ay));
   else if(r>1.0) t=sqrt((cx-bx)*(cx-bx)+(cy-by)*(cy-by));
   else return FLOAT_TO_FIXED(s);
   if(s>0.0) return FLOAT_TO_FIXED(t);
   return -FLOAT_TO_FIXED(t);
};

/*static void wall_vector(LevData *ld, int wall, fixed *vx, fixed *vy)
{
   VertexDyn *vd1=ldvertexd(ld)+ldline(ld)[wall].ver1;
   VertexDyn *vd2=ldvertexd(ld)+ldline(ld)[wall].ver2;
   fixed dx = vd2->x-vd1->x;
   fixed dy = vd2->y-vd1->y;
   fixed ll=fix_pythagoras(dx,dy);
   
   *vx = fixdiv(dx,ll);
   *vy = fixdiv(dy,ll);
};*/

static fixed wall_angle(const LevData *ld, int wall)
{
   VertexDyn *vd1=ldvertexd(ld)+ldline(ld)[wall].ver1;
   VertexDyn *vd2=ldvertexd(ld)+ldline(ld)[wall].ver2;
   fixed dx = vd2->x-vd1->x;
   fixed dy = vd2->y-vd1->y;
   return fix_vec2angle(dx,dy);
};

static const LE_int16 *get_blockmap(const LevData *ld,fixed x,fixed y)  {
   const BlockMapHdr *h=&(ldblockmap(ld)->UMEMB(hdr));
   x-=h->minx<<12;
   y-=h->miny<<12;
   if(x<0||y<0) return NULL;
   x/=BM_BLOCKSIZE<<12;
   y/=BM_BLOCKSIZE<<12;
   if(x>=h->numx||y>=h->numy) return NULL;
   return ldblockmap(ld)->data+h->idx[x+y*h->numx];
};
static void get_blockmaps(const LE_int16 **maps,
			  const LevData *ld,fixed x,fixed y,fixed r)  {
   signed char ox,oy; 
   const LE_int16 **map=maps,**m;
   for(oy=-1;oy<=1;oy++) for(ox=-1;ox<=1;ox++) {
      *map=get_blockmap(ld,x+ox*r,y+oy*r);
      for(m=maps;m<map&&*map!=NULL;m++) if(*m==*map) *map=NULL;
      if(*map) map++;
   };
   *map=NULL;
};

extern inline double pyth_sq(fixed x,fixed y) {
   double dx=FIXED_TO_FLOAT(x),dy=FIXED_TO_FLOAT(y);
   return dx*dx+dy*dy;
}; 
#define SQ(x) ((x)*(x))


void thing_chk_collide(LevData *ld,int thingnum) {
   ThingDyn *td=ldthingd(ld)+thingnum;
   double f;
   int on;
	 /* now look for object collisions */
	 for(on=0;on<ldnthings(ld);on++)  {
	    ThingDyn *otd=ldthingd(ld)+on;
	    if(td==otd) continue;
	    if(otd->proto==NULL) continue;
	    if(otd->proto->flags&PT_PHANTOM) continue;
	    if(otd->proto->height+otd->z<td->z) continue;
	    if(otd->z>td->proto->height+td->z) continue;
	    if(abs(otd->x-td->x)>td->proto->radius+otd->proto->radius) 
	       continue;
	    if(abs(otd->y-td->y)>td->proto->radius+otd->proto->radius) 
	       continue;
	    f=pyth_sq(otd->x-td->x,otd->y-td->y);
	    /* are objects overlapping? */
	    if(f<=SQ(FIXED_TO_FLOAT(td->proto->radius+otd->proto->radius)))  {
	       /* explode, if necessary */
	       if(td->proto->flags&PT_EXPLOSIVE) {
		  /* missiles don't explode each other */
		  if(otd->proto->flags&PT_EXPLOSIVE) continue;
		  /* can't collide with your own missiles */
		  /* they can still hurt you when they explode though */
		  if(on==td->owner) continue;
		  /* stop and explode */
		  td->dx=td->dy=td->dz=0;
		  thing_send_sig(ld,thingnum,TS_EXPLODE);
		  /*logprintf(LOG_DEBUG,'O',"missile hit %d: dist=%f",on,
			    sqrt(f));*/
	       }
	       /* are we a player running into a gettable */
	       else if((td->proto->flags&PT_PLAYER)&&otd->proto->artitype>=0) {
		  int pl=thing2player(ld,thingnum);
		  if(pl>=0) pickup_gettable(ld,pl,
					    otd->proto->artitype,
					    otd->proto->artinum);
		  otd->proto=NULL;
	       }
	       /* if object is blocking, stop */
	       else if(otd->proto->flags&PT_BLOCKING) {
		  td->dx=td->dy=td->dz=0;
		  break;
	       };
	    };
	 };
};

void update_things(LevData *ld,int tickspassed)  {
   int thingnum;
   ThingDyn *td=ldthingd(ld);
   int wall;
   int steps,i;
   fixed r,s;
   double f;

   for(thingnum=0;thingnum<ldnthings(ld);thingnum++,td++)  {
      fixed dx,dy,dz,dangle;
      /* maximum floor, min ceiling this thing is on */
      fixed maxfloor=FIXED_MIN,minceiling=FIXED_MAX;
      
      /* number of walls this thing is sliding along */
      int slide_walls=0;

      int walls_crossed=0;
      
      /* check that thing exists */
      if(td->proto==NULL) continue;
      
      /* update thing state machine */
      td->phase_wait-=tickspassed;
      while((td->pending_signal!=TS_NOSIG||td->phase_wait<=0)&&td->proto)  {
	 int next;
	 if(td->pending_signal!=TS_NOSIG) 
	    next=td->proto->signals[td->pending_signal];
	 else 
	    next=td->phase_tbl[td->phase].next;
	 td->pending_signal=TS_NOSIG;
	 if(next<0) next=td->phase+1;
	 thing_enter_phase(ld,thingnum,next);
      };
      
      /* maybe thing has stopped existing now */
      if(td->proto==NULL) continue;

      /* deal with bogosities */
      if(td->proto->flags&PT_BOGUS) {
	 const ThingDyn *p;
	 p=ldthingd(ld)+td->owner;
	 td->x=p->x;
	 td->y=p->y;
	 td->z=p->z;
	 td->angle=p->angle;
	 td->elev=p->elev;
	 continue;
      };
      
      /* or has fallen off the map somehow */
      if(td->sector<0) continue;

      /* check if thing is static */
      if(td->proto->flags&PT_INF_MASS) {
	 if(td->sector>=0&&td->proto->flags&PT_HANGING) 
	       td->z=ldsectord(ld)[td->sector].ceiling-td->proto->height;
	 continue;
      };

      /* apply gravity */
      if(td->proto->realmass==0||(td->proto->flags&PT_CAN_FLY));
      else if(td->proto->flags&PT_FLOATSUP)
	 td->dz=(td->proto->speed*tickspassed)/TICKDIV;
      else if(td->dz<-FIXED_EPSILON)
	 td->dz-=(2*GRAVITY*tickspassed)/TICKDIV;
      else
	 td->dz-=(GRAVITY*tickspassed)/TICKDIV;
      
      /* fake up velocities based on tickspassed */
      dx=(td->dx*tickspassed)/TICKDIV;
      dy=(td->dy*tickspassed)/TICKDIV;
      dz=(td->dz*tickspassed)/TICKDIV;
      dangle=(td->dangle*tickspassed)/TICKDIV;

      /* phantoms never worry about collision */
      if(td->proto->flags&PT_PHANTOM)  {
	 td->x+=dx;
	 td->y+=dy;
	 td->z+=dz;
	 td->angle+=dangle;
	 continue;
      };

#if 0
      /* don't move more than our height vertically */
      r=td->proto->height;
      if(dz>=r) dz=r-1;
      if(dz<=-r) dz=1-r;
#endif

      /* if we're moving more than our radius horizontally, do it in steps */
      r=td->proto->radius;
      f=pyth_sq(dx,dy);
      if(f>SQ(FIXED_TO_FLOAT(r)))  {
	 s=FLOAT_TO_FIXED(sqrt(f));
	 s=FIXED_TRUNC(fixdiv(s,r))+FIXED_ONE;
	 dx=fixdiv(dx,s);
	 dy=fixdiv(dy,s);
	 steps=FIXED_TO_INT(s);
      }
      else steps=1;
	 
      for(i=0;i<steps;i++) {
#ifndef NO_BLOCKMAP
	 const LE_int16 *walltbl[10],**wallptr;
#endif      
	 /* look for impassible walls coming within my radius */
	 r=td->proto->radius;
	 /* The smallest circle that can contain the viewplane has 
	  * radius sqrt(2).  If an object with a view gets closer than this
	  * to a visible wall, the viewplane and the wall intersect (bad) */
	 if(r<FIXED_ROOT2) r=FIXED_ROOT2;

#ifndef NO_BLOCKMAP
	 get_blockmaps(walltbl,ld,td->x,td->y,r);
	 for(wallptr=walltbl;*wallptr;wallptr++) while((wall=*(*wallptr)++)>=0)
#else	 
	 for(wall=0;wall<ldnlines(ld);wall++)
#endif	    
	 {
	    fixed tx,ty;
	    fixed dist,tdist;
	    fixed step_height,jump_height,climb_height;
	    fixed headroom;
	    short front,back;
	    
	    /* if I'm no longer moving, bail out */
	    if(dx==0&&dy==0) break;
	    
	    /* length 0 walls cause division by zero & can safely be ignored */
	    if(wall_length(ld,wall)==0) continue;
	    
	    /* the "target" position we're ending up in */
	    tx=td->x+dx;
	    ty=td->y+dy;
	    
	    /* how far from the wall are we now? */ 
	    dist=line_dist(ld,wall,td->x,td->y);
	    if(dist>r*2) continue;
	    
	    /* find sector on our side (front) and other side (back) */
	    if(dist>0) {
	       front=ldline(ld)[wall].side[1];
	       back=ldline(ld)[wall].side[0];
	    }
	    else {
	       front=ldline(ld)[wall].side[0];
	       back=ldline(ld)[wall].side[1];
	    };
	    if(front>=0) front=ldside(ld)[front].sector;
	    if(back>=0) back=ldside(ld)[back].sector;
	    if(front<0) continue;
	    
	    /* find useful values for this wall */
	    if(back<0 || (ldline(ld)[wall].flags&LINE_IMPASSIBLE))  {
	       headroom=FIXED_MIN;
	       step_height=FIXED_MAX;
	    }
	    else  {
	       step_height=ldsectord(ld)[back].floor-
		  ldsectord(ld)[front].floor;
	       headroom=ldsectord(ld)[back].ceiling;
	    };
	    
	    /* object's height above its sector */
	    jump_height=td->z-ldsectord(ld)[front].floor;

	    /* maximum height we can climb */
	    if((td->proto->flags&PT_CAN_CLIMB)&&td->hits>0) 
	      climb_height=td->proto->height/2;
	    else 
	      climb_height=0;
	    
	    /* how far will we be when we've moved? */
	    tdist=line_dist(ld,wall,tx,ty);
	    
	    /* is this wall too far away to be of interest? */
	    if(FIXED_ABS(tdist)>r&&FIXED_ABS(dist)>r) continue;

	    /* keep track of how far we can fall */
	    if(FIXED_ABS(dist)<=r) {
	       if(ldsectord(ld)[front].floor>maxfloor) 
		  maxfloor=ldsectord(ld)[front].floor;
	       if(back>=0&&ldsectord(ld)[back].floor>maxfloor)
		  maxfloor=ldsectord(ld)[back].floor;

	       /* and how high we can leap */
	       if(ldsectord(ld)[front].ceiling<minceiling) 
		  minceiling=ldsectord(ld)[front].ceiling;
	       if(back>=0&&ldsectord(ld)[back].ceiling<minceiling)
		  minceiling=ldsectord(ld)[back].ceiling;
	    };

	    /* is this wall passable? */
	    if(step_height<=jump_height+climb_height&&
	       td->z+td->proto->height<=headroom&&
	       !((td->proto->flags&PT_SKIRT_CLIFFS)&&
		 step_height<-climb_height&&td->hits>0)
	       )  {
	       
	       /* are we actually going to cross it? if not, ignore it */
	       if(!FIXED_PRODUCT_SIGN(tdist,dist)) continue;
	       /* manipulate view when climbing or descending */
	       if((td->proto->flags&PT_BEASTIE)&&jump_height<=climb_height) {
		  if(step_height>6<<12) td->delev+=FIXED_ONE/32;
		  else if(step_height<-(6<<12)) td->delev-=FIXED_ONE/32;
		  else if(step_height>10<<12) td->delev+=FIXED_ONE/24;
		  else if(step_height<-(10<<12)) td->delev-=FIXED_ONE/24;
	       };
	       /* take any "action on crossed" action for this wall */
	       thing_hit_wall(ld,thingnum,wall,dist>0,Crossed);
	       walls_crossed++;
	       continue;
	    };
	    
	    /* wall must be impassible */
	 
	    /* check if wall is within our radius */
	    if(FIXED_ABS(tdist)<=r) {

	       /* if explosive, stop now and explode */
	       if(td->proto->flags&PT_EXPLOSIVE)  {
		  dz=dx=dy=td->dx=td->dy=td->dz=0;
		  thing_send_sig(ld,thingnum,TS_EXPLODE);
		  thing_hit_wall(ld,thingnum,wall,dist>0,Damaged);
	       }
	       /* slide along wall
		* two things to know about this:
		* 1) the idea is that we change velocity to be
		*    parallel to the wall (langle), with magnitude equal to
                *    the wall-parallel component of the old velocity vector.
		*    Conveniently, this will always be less than the
		*    magnitude of the old velocity, so sliding along several 
		*    walls will eventually slow us down to a halt.
		* 2) Under some circumstances (imagine running into the 
		*    common vertex of two walls stretching away from you)
		*    this won't work.  We can't make up our mind which
		*    wall to slide along, and just keep dithering until we run
		*    out of momentum.  Where the cosine of the angle between 
		*    the walls (which controls how much sliding slows us down)
		*    is near to 1, this is as good as an infinite loop.
		*    Thus, if we
		*    haven't decided which way to go after SLIDE_BAILOUT
		*    walls, we give up and stop in our tracks. 
		*/
	       else  {
		  fixed myangle=fix_vec2angle(dx,dy);
		  fixed langle=wall_angle(ld,wall);
		  fixed newmv,oldmv;
		  fixed angdiff=myangle-langle;
		  /*int onfront=dist>0;*/
		  /*logprintf(LOG_DEBUG,'O',
			    "sliding along wall %d dst=%f tdst=%f angdiff=%f",
			    wall,
			    FIXED_TO_FLOAT(dist),
			    FIXED_TO_FLOAT(tdist),
			    FIXED_TO_FLOAT(angdiff));*/
		  if((angdiff<-FIXED_PI||(angdiff>0&&angdiff<FIXED_PI))&&
		     ldline(ld)[wall].side[1]<0);
		  else {
		     oldmv=fix_pythagoras(td->dx,td->dy);
		     newmv=fixmul(oldmv,fixcos(angdiff));
		     td->dx=td->dy=0;
		     /* take any "action on thumped" action for this wall */
		     thing_hit_wall(ld,thingnum,wall,dist>0,Thumped);
		     if(++slide_walls<SLIDE_BAILOUT) {
			thingd_apply_polar(td,newmv,langle);
			/* we've changed dx & dy, 
			   need to check all walls again now */
			wall=-1;
			/* and we need to recalc the faked d? vals */
			dx=((td->dx*tickspassed)/TICKDIV)/steps;
			dy=((td->dy*tickspassed)/TICKDIV)/steps;
		     };
		  };
	       };
	    };
	 };

	 if(dx==0&&dy==0) break;
	 /* update plane position */
	 td->x+=dx;
	 td->y+=dy;
	 if(walls_crossed>0) thingd_findsector(ld,td);
	 
	 thing_chk_collide(ld,thingnum);
	 if(td->dx==0&&td->dy==0) {
	    dx=dy=0;
	    break;
	 };
      
      /*end of "steps" loop */
      };
      
      /* now adjust dx and dy back to what they were before "steps" */
      dx*=steps;
      dy*=steps;
      
      /* check that I don't fall through floor, or rise through ceiling */
      if(ldsectord(ld)[td->sector].floor>maxfloor) 
	 maxfloor=ldsectord(ld)[td->sector].floor;
      if(dz+td->z<maxfloor) {
	 if(td->proto->flags&PT_EXPLOSIVE)  {
	    dz=dx=dy=td->dx=td->dy=td->dz=0;
	    thing_send_sig(ld,thingnum,TS_EXPLODE);
	 }
	 else if(td->proto->flags&PT_BOUNCY) {
	    dz=maxfloor-td->z;
	    td->dz=td->bouncemax;
	    td->bouncemax/=2;
	 }
	 else
	    dz=maxfloor-td->z;
      };
      /* ceiling gets priority so crushers grind you into the ground */
      if(ldsectord(ld)[td->sector].ceiling<minceiling)
	 minceiling=ldsectord(ld)[td->sector].ceiling;
      r=minceiling-td->proto->height;
      if(dz+td->z>=r) {
	 if(td->proto->flags&PT_EXPLOSIVE)  {
	    dz=dx=dy=td->dx=td->dy=td->dz=0;
	    thing_send_sig(ld,thingnum,TS_EXPLODE);
	 }
	 else {
	    dz=r-(td->z+FIXED_EPSILON);
	    sector_crush_thing(ld,td->sector,thingnum);
	 };
      };
   
      /* update angular and vertical position */
      td->z+=dz;
      td->angle+=dangle;
      NORMALIZE_ANGLE(td->angle);

      /* update velocities */
      if(td->proto->friction!=FIXED_ONE) {
	 r=td->proto->friction*1024;
	 for(i=1;i<tickspassed;i++) r=fixmul(r,td->proto->friction);
	 td->dx=fixmul(td->dx/1024,r);
	 td->dy=fixmul(td->dy/1024,r);
	 td->dz=fixmul(td->dz/1024,r);
	 td->dangle=fixmul(td->dangle/1024,r);
      };

      /* now update elev */
      td->elev+=td->delev;
      /* update delev */
      td->delev=td->delev/2;

   };
};
