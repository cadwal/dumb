
#ifndef ENDIANTYPES_H
#define ENDIANTYPES_H

#ifdef CHAR_BY_CHAR_BYTESWAP
/* this might eliminate misalignment problems on bigendian machines */ 
/* shouldn't be necessary now */

typedef union {unsigned int i;unsigned char c[4];} EInt32;
typedef union {unsigned short i;unsigned char c[2];} EInt16;

#define INT32_CHAR(x,n) ( (unsigned int)(((EInt32*)&(x))->c[n]) )
#define INT16_CHAR(x,n) ( (unsigned int)(((EInt16*)&(x))->c[n]) )
#define FROM_LE32(x) ( INT32_CHAR(x,0)|(INT32_CHAR(x,1)<<8)|(INT32_CHAR(x,2)<<16)|(INT32_CHAR(x,3)<<24) )
#define FROM_LE16(x) ( INT16_CHAR(x,0)|(INT16_CHAR(x,1)<<8) )

#else /* !CHAR_BY_CHAR_BYTESWAP */

#define SWAPSHORT(i) (((i>>8)&0xff)|((i<<8)&0xff00))

#define SWAPINT(i) (((i>>24)&0xff)|((i>>8)&0xff00)|((i<<8)&0xff0000)|((i<<24)&0xff000000))

#ifdef WORDS_BIGENDIAN
#define FROM_BE16(x) 
#define FROM_LE16(x) SWAPSHORT((x))
#define FROM_BE32(x) 
#define FROM_LE32(x) SWAPINT((x))
#else  /* !WORDS_BIGENDIAN */
#define FROM_LE16(x) 
#define FROM_BE16(x) SWAPSHORT((x))
#define FROM_LE32(x) 
#define FROM_BE32(x) SWAPINT((x))
#endif /* !WORDS_BIGENDIAN */

#endif /* !CHAR_BY_CHAR_BYTESWAP */

#ifdef WORDS_BIGENDIAN

#ifndef __cplusplus
/* Commented out because __cplusplus is not defined when preprocessing
 * for dependencies.  And if someone tries to compile this without C++,
 * there will be enough error messages even without the extra check.
 *
 * #error "You need to compile DUMB with C++ to support big endian processors"
 */
#else /* __cplusplus */

#define IA(o) \
int operator o##=(int j) {i o##= FROM_LE32(j); return FROM_LE32(i);}
#define SA(o) \
short operator o##=(short j) {i o##= FROM_LE16(j); return FROM_LE16(i);}

#define IB(o) \
int operator o##=(int j) {*this=((int)*this) o j; return (int)*this;}
#define IBS IB(+) IB(-) IB(*) IB(/) \
int operator ++() {return *this+=1;} \
int operator --() {return *this-=1;} \
int operator ++(int d) {d=*this; *this+=1; return d;} \
int operator --(int d) {d=*this; *this-=1; return d;}

class LE_flags32 {
   int i;
public:
   LE_flags32() {}
   LE_flags32(int j) :i(FROM_LE32(j)) {}
   operator int() const {return FROM_LE32(i);}
   int operator=(int j) {i=FROM_LE32(j); return j;}
   int operator&(int j) const {return i&FROM_LE32(j);}
   IA(^) IA(|) IA(&)
};

class LE_int32 {
   int i;
public:
   LE_int32() {}
   LE_int32(int j) :i(FROM_LE32(j)) {}
   operator int() const {return FROM_LE32(i);}
   int operator=(int j) {i=FROM_LE32(j); return j;}
   IBS
};

class LE_flags16 {
   short i;
public:
   LE_flags16() {}
   LE_flags16(short j) :i(FROM_LE16(j)) {}
   operator short() const {return FROM_LE16(i);}
   short operator=(short j) {i=FROM_LE16(j); return j;}
   int operator&(short j) const {return i&FROM_LE16(j);}
   SA(^) SA(|) SA(&)
};

class LE_int16 {
   short i;
public:
   LE_int16() {}
   LE_int16(short j) :i(FROM_LE16(j)) {}
   operator short() const {return FROM_LE16(i);}
   short operator=(short j) {i=FROM_LE16(j); return j;}
   IBS
};

class LE_uint32 {
   int i;
public:
   LE_uint32() {}
   LE_uint32(unsigned int j) :i(FROM_LE32(j)) {}
   operator unsigned int() const {return FROM_LE32(i);}
   unsigned int operator=(unsigned int j) {i=FROM_LE32(j); return j;}
   IBS
};

class LE_uint16 {
   short i;
public:
   LE_uint16() {}
   LE_uint16(unsigned short j) :i(FROM_LE16(j)) {}
   operator unsigned short() const {return FROM_LE16(i);}
   unsigned short operator=(unsigned short j) {i=FROM_LE16(j); return j;}
   IBS
};

#undef SA
#undef IA

#endif /* __cplusplus */

#else /* !WORDS_BIGENDIAN */

typedef int LE_int32;
typedef int LE_flags32;
typedef short LE_int16;
typedef short LE_flags16;
typedef unsigned int LE_uint32;
typedef unsigned short LE_uint16;

#endif /* !WORDS_BIGENDIAN */

#endif
