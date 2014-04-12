/*
 * asm.h - Definició de variable global als fitxers .s
 */

#ifndef __ASM_H__
#define __ASM_H__

#define ENTRY(name) \
  .globl name; \
    .type name, @function; \
    .align 0; \
  name:

#define MAX_SYSCALL 40

#endif  /* __ASM_H__ */
