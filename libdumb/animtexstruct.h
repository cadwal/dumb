#ifndef ANIMTEXSTRUCT_H
#define ANIMTEXSTRUCT_H

typedef struct {
   unsigned char seqlen;
   unsigned char myseqnum;
   char name[9];
   unsigned char flags;
   int duration,_spare;
} AnimTexTable;

#define AT_FLAT 0x0001
#define AT_SWITCH 0x0002

#endif
