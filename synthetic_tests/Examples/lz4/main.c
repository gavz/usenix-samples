#include<stdio.h>
#include<stdlib.h>

//void unlz4_len(const void *aSource, void *aDestination, uint32_t aLength);
//void unlz4(const void *aSource, void *aDestination);
//void getLength();
//void copyData();
__attribute__((naked))
void bunlz4(const void *aSource, void *aDestination){
  __asm volatile("unlz4:         ldrh         r2,[r0]");        /* get length of compressed data */
  __asm volatile("               adds         r0,r0,#2");       /* advance source pointer */
}

__attribute__((naked))
void aunlz4_len(const void *aSource, void *aDestination, uint32_t aLength){
  __asm volatile("unlz4_len:     push         {r4-r6,lr}");     /* save r4, r5, r6 and return-address */
  __asm volatile("               adds         r5,r2,r0");       /* point r5 to end of compressed data */
  __asm volatile("getToken:      ldrb         r6,[r0]");        /* get token */
  __asm volatile("               adds         r0,r0,#1");       /* advance source pointer */
  __asm volatile("               lsrs         r4,r6,#4");       /* get literal length, keep token in r6 */
  __asm volatile("               beq          getOffset");      /* jump forward if there are no literals */
  __asm volatile("               bl           getLength");      /* get length of literals */
  __asm volatile("               movs         r2,r0");          /* point r2 to literals */
  __asm volatile("               bl           copyData");       /* copy literals (r2=src, r1=dst, r4=len) */
  __asm volatile("               movs         r0,r2");          /* update source pointer */
  __asm volatile("getOffset:     ldrb         r3,[r0,#0]");     /* get match offset's low byte */
  __asm volatile("               subs         r2,r1,r3");       /* subtract from destination; this will become the match position */
  __asm volatile("               ldrb         r3,[r0,#1]");     /* get match offset's high byte */
  __asm volatile("               lsls         r3,r3,#8");       /* shift to high byte */
  __asm volatile("               subs         r2,r2,r3");       /* subtract from match position */
  __asm volatile("               adds         r0,r0,#2");       /* advance source pointer */
  __asm volatile("               lsls         r4,r6,#28");      /* get rid of token's high 28 bits */
  __asm volatile("               lsrs         r4,r4,#28");      /* move the 4 low bits back where they were */
  __asm volatile("               bl           getLength");      /* get length of match data */
  __asm volatile("               adds         r4,r4,#4");       /* minimum match length is 4 bytes */
  __asm volatile("               bl           copyData");       /* copy match data (r2=src, r1=dst, r4=len) */
  __asm volatile("               cmp          r0,r5");          /* check if we've reached the end of the compressed data */
  __asm volatile("               blt          getToken");       /* if not, go get the next token */
  __asm volatile("               pop          {r4-r6,pc}");     /* restore r4, r5 and r6, then return */
}

__attribute__((naked))
void getLength(){
  __asm volatile("getLength:     cmp          r4,#0x0f");       /* if length is 15, then more length info follows */
  __asm volatile("               bne          gotLength");      /* jump forward if we have the complete length */
  __asm volatile("getLengthLoop: ldrb         r3,[r0]");        /* read another byte */
  __asm volatile("               adds         r0,r0,#1");       /* advance source pointer */
  __asm volatile("               adds         r4,r4,r3");       /* add byte to length */
  __asm volatile("               cmp          r3,#0xff");       /* check if end reached */
  __asm volatile("               beq          getLengthLoop "); /* if not, go round loop */
  __asm volatile("gotLength:     bx           lr");             /* return */
}

__attribute__((naked))
void copyData(){
  __asm volatile("copyData:      rsbs         r4,r4,#0");       /* index = -length */
  __asm volatile("               subs         r2,r2,r4");       /* point to end of source */
  __asm volatile("               subs         r1,r1,r4");       /* point to end of destination */
  __asm volatile("copyDataLoop:  ldrb         r3,[r2,r4]");     /* read byte from source_end[-index] */
  __asm volatile("               strb         r3,[r1,r4]");     /* store byte in destination_end[-index] */
  __asm volatile("               adds         r4,r4,#1");       /* increment index */
  __asm volatile("               bne          copyDataLoop");   /* keep going until index wraps to 0 */
  __asm volatile("               bx           lr");             /* return */
}
void main(void){
  unsigned char sMyCompressedData[44] = {
 	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x22, 0x4d, 0x18, 0x64, 
 	0x40, 0xa7, 0x0d, 0x00, 0x00, 0x80, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x77, 0x6f, 0x72, 0x6c, 
 	0x64, 0x21, 0x0a, 0x00, 0x00, 0x00, 0x00, 0xe8, 0x1e, 0x4b, 0x08, 
  };

  unsigned char uncompressedData[100];



  aunlz4_len(&sMyCompressedData[0],&uncompressedData[0],sizeof(sMyCompressedData));
  #ifdef KLEE
  printf("uncompressed data:\n%s",uncompressedData);
  #endif

}


