
#ifndef ENDIAN_H
#define ENDIAN_H

#define SWAPSHORT(i) (((i>>8)&0xff)|((i<<8)&0xff00))

#define SWAPINT(i) (((i>>24)&0xff)|((i>>8)&0xff00)|((i<<8)&0xff0000)|((i<<24)&0xff000000))

#ifdef BENDIAN
#define FROM_BE16(x) 
#define FROM_LE16(x) (x)=SWAPSHORT((x))
#define FROM_BE32(x) 
#define FROM_LE32(x) (x)=SWAPINT((x))
#else
#define FROM_LE16(x) 
#define FROM_BE16(x) (x)=SWAPSHORT((x))
#define FROM_LE32(x) 
#define FROM_BE32(x) (x)=SWAPINT((x))
#endif

#if defined(__cplusplus)&&defined(BENDIAN)

#define IA(o) \
int operator o##=(int j) {i o##= SWAPINT(j); return SWAPINT(i);};
#define SA(o) \
short operator o##=(short j) {i o##= SWAPSHORT(j); return SWAPSHORT(i);};

#define IB(o) \
int operator o##=(int j) {*this=((int)*this) o j; return (int)i;}
#define IBS IB(+) IB(-) IB(*) IB(/) \
int operator ++() {return *this+=1;}; \
int operator --() {return *this-=1;}; \
int operator ++(int d) {d=*this; *this+=1; return d;}; \
int operator --(int d) {d=*this; *this-=1; return d;};

class LE_flags32 {
   int i;
public:
   LE_flags32() {};
   LE_flags32(int j) :i(SWAPINT(j)) {};
   operator int() const {return SWAPINT(i);};
   int operator=(int j) {i=SWAPINT(j); return j;};
   int operator&(int j) const {return i&SWAPINT(j);};
   IA(^) IA(|) IA(&)
};

class LE_int32 {
   int i;
public:
   LE_int32() {};
   LE_int32(int j) :i(SWAPINT(j)) {};
   operator int() const {return SWAPINT(i);};
   int operator=(int j) {i=SWAPINT(j); return j;};
   IBS
};

class LE_flags16 {
   short i;
public:
   LE_flags16() {};
   LE_flags16(short j) :i(SWAPSHORT(j)) {};
   operator short() const {return SWAPSHORT(i);};
   short operator=(short j) {i=SWAPSHORT(j); return j;};
   int operator&(short j) const {return i&SWAPSHORT(j);};
   SA(^) SA(|) SA(&)
};

class LE_int16 {
   short i;
public:
   LE_int16() {};
   LE_int16(short j) :i(SWAPSHORT(j)) {};
   operator short() const {return SWAPSHORT(i);};
   short operator=(short j) {i=SWAPSHORT(j); return j;};
   IBS
};

class LE_uint32 {
   int i;
public:
   LE_uint32() {};
   LE_uint32(unsigned int j) :i(SWAPINT(j)) {};
   operator unsigned int() const {return SWAPINT(i);};
   unsigned int operator=(unsigned int j) {i=SWAPINT(j); return j;};
   IBS
};

class LE_uint16 {
   short i;
public:
   LE_uint16() {};
   LE_uint16(unsigned short j) :i(SWAPSHORT(j)) {};
   operator unsigned short() const {return SWAPSHORT(i);};
   unsigned short operator=(unsigned short j) {i=SWAPSHORT(j); return j;};
   IBS
};

#undef SA
#undef IA

#else

#ifdef BENDIAN
#error "You need to compile DUMB with C++ to support big endian processors!"
#endif

typedef int LE_int32;
typedef int LE_flags32;
typedef short LE_int16;
typedef short LE_flags16;
typedef unsigned int LE_uint32;
typedef unsigned short LE_uint16;

#endif

#endif
