
#ifndef FIXED_H
#define FIXED_H

/* fixed point conversions */
#define INT_TO_FIXED(i) ((i) << 16)
#define FIXED_TO_INT(f) ((f) >> 16)
#define FIXED_TO_INT_UP(f) (((f)+0xffff) >> 16)
#define FIXED_TO_FLOAT(f) (((double) (f)) * 1.52587890625e-5)
#define FLOAT_TO_FIXED(f) ((fixed) ((f) * 65536.0))

/* functions */
#define FIXED_ABS(f) ((f) < 0 ? -(f) : (f))
#define FIXED_TRUNC(f) ((f) & 0xffff0000)
#define FIXED_SIGN(f) ((unsigned int) (f) >> 31)
#define FIXED_PRODUCT_SIGN(f, g) ((unsigned int) ((f) ^ (g)) >> 31)
#define FIXED_HALF(f) ((f) / 2)
#define FIXED_DOUBLE(f) ((f) << 1)

/* perform integer scaling of a fixed point number */
#define FIXED_SCALE(f, i) ((f) * (i))


/* fixed point constants */
#define FIXED_ZERO     (INT_TO_FIXED(0))
#define FIXED_ONE      (INT_TO_FIXED(1))
#define FIXED_ONE_HALF (FIXED_HALF(FIXED_ONE))
#define FIXED_PI       (FLOAT_TO_FIXED(3.14159265))
#define FIXED_2PI      (FLOAT_TO_FIXED(6.28318531))
#define FIXED_HALF_PI  (FLOAT_TO_FIXED(1.57079633))
#define FIXED_ROOT2    (FLOAT_TO_FIXED(1.4142))
#define FIXED_MIN      INT_MIN
#define FIXED_MAX      INT_MAX

/* we need this for kludges to avoid fixed point division overflow */
#define FIXED_EPSILON  ((fixed) 0x100)

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

typedef int fixed;

#if !defined(__GNUC__)
#define FIX_ATTR
#elif defined(i386)
#define FIX_ATTR __attribute__ ((const, regparm (3)))
#else
#define FIX_ATTR __attribute__ ((const))
#endif

fixed fix_sqrt(fixed) FIX_ATTR;
fixed fix_pythagoras(fixed x,fixed y) FIX_ATTR;
fixed fix_pyth3d(fixed x,fixed y,fixed z) FIX_ATTR;
fixed fix_vec2angle(fixed x,fixed y) FIX_ATTR;

#define NORMALIZE_ANGLE(x) while(x>=FIXED_2PI) x-=FIXED_2PI; while(x<0) x+=FIXED_2PI

fixed fixsin(fixed) FIX_ATTR;
fixed fixcos(fixed) FIX_ATTR;
fixed fixtan(fixed) FIX_ATTR;

#define FIXSIN(x) FLOAT_TO_FIXED(sin(FIXED_TO_FLOAT(x)))
#define FIXCOS(x) FLOAT_TO_FIXED(cos(FIXED_TO_FLOAT(x)))
#define FIXTAN(x) FLOAT_TO_FIXED(tan(FIXED_TO_FLOAT(x)))

#if defined(__GNUC__) && defined(i386)

/*********************** Intel fixed point functions **********************/

static inline fixed fixmul(fixed r1, fixed r2)
{
     fixed result;

     __asm__ ("imull %2\n\t"
	      "shrd  $16, %%edx, %%eax\n\t"
	      :"=a" (result)
	      :"%a" (r1), "mr" (r2): "edx");

     return result;
}


static inline fixed fixmul2_30(fixed r1, fixed r2)
{
     fixed result;

     __asm__ ("imull %2\n\t"
	      "shrd  $30, %%edx, %%eax\n\t"
	      :"=a" (result)
	      :"%a" (r1), "mr" (r2): "edx");

     return result;
}


static inline fixed fixdiv(fixed dividend, fixed divisor)
{
     fixed result;

     __asm__("sar  $16, %%edx\n\t"
	     "idivl %3, %%eax\n\t"
	     :"=a" (result)
	     :"a" (dividend<<16), "d" (dividend), "mr" (divisor)
	     : "edx");

     return result;
}

#elif defined(ARCH_MAC) && !defined(__powerc)

/******************** MBW - 68k Macintosh only... ****************/

/* toolbox trap versions - slow... */
/* extern pascal fixed fixmul(fixed a, fixed b) = 0xA868; */
/* extern pascal fixed fixmul2_30(fixed x, fixed y) = 0xA84A; */

/* inline versions */
#pragma parameter __D0 fixmul(__D0,__D1)
fixed fixmul(fixed a, fixed b) = 
{
        0x4C01,0x0C01   // muls.L D1,D1:D0
        ,0x3001                 // move.w D1,D0
        ,0x4840                 // swap D0
};
#pragma parameter __D1 fixmul2_30(__D0,__D1)
fixed fixmul2_30(fixed a, fixed b) = 
{
        0x4C01,0x0C01   // muls.L D1,D1:D0
        ,0xE589                 // lsl.L #2, D1
        /* without the following lines, we're dropping the 
           least signifigant 2 bits.  If this is a problem, 
           uncomment them. 
         */
         //,0xE598                       // rol.L #2, D0
         //,0xEFC1, 0x0782       // bfins D0, D1{30,2}
};

#undef INT_TO_FIXED
#undef FIXED_TO_INT
#undef FIXED_TRUNC

#pragma parameter __D0 FIXED_TRUNC(__D0)
fixed FIXED_TRUNC(fixed x) = 
        0x4240;         // clr.W D0

#pragma parameter __D0 FIXED_TO_INT(__D0)
signed short FIXED_TO_INT(fixed x) = 
        0x4840;         // swap D0

#pragma parameter __D0 INT_TO_FIXED(__D0)
fixed INT_TO_FIXED(signed short x) = 
{
        0x4840          // swap D0
        ,0x4240         // clr.w D0
};

/* MBW - one of these days I'll rewrite division too. 
   The toolbox trap works, though.
*/
extern pascal fixed fixdiv(fixed x, fixed y) = 0xA84D;

#else

/****************** Generic fixed point functions ********************/

#define FIXMUL_AS_MACRO

#ifdef NO_LONGLONG
#define fixmul(a, b) ((fixed) ((double) (a) * ((double) (b)) * \
			       1.52587890625e-5))
#define fixmul2_30(a, b) ((fixed) ((double) (a) * ((double) (b) * \
				    9.313225746154785e-10)))
#else
#define fixmul(a,b)  ( (fixed) (((long long)(a)*(long long)(b)) >> 16) )
#define fixmul2_30(a,b)  ( (fixed) (((long long)(a)*(long long)(b)) >> 30) )
#endif

#define fixdiv(a, b) ((fixed) (((double) (a) / (double) (b)) * 65536.0))

#endif


#endif




