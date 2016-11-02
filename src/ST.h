#include <stdint.h>
#define NULL ((void *)0)

#if 0
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;

typedef int32_t intptr_t;
typedef uint32_t uintptr_t;
#endif

#define VID_BASE_H (*(volatile uint8_t *)0xFF8201)
#define VID_BASE_M (*(volatile uint8_t *)0xFF8203)
#define VID_CUR_H (*(volatile uint8_t *)0xFF8205)
#define VID_CUR_M (*(volatile uint8_t *)0xFF8207)
#define VID_CUR_L (*(volatile uint8_t *)0xFF8209)
#define VID_SHIFT_CLK (*(volatile uint8_t *)0xFF820A)
#define VID_BASE_L_STE (*(volatile uint8_t *)0xFF820D)
#define VID_STRIDE_STE (*(volatile uint8_t *)0xFF820F)
#define VID_STRIDE_FALCON (*(volatile uint16_t *)0xFF8210)
#define VID_PAL0 ((volatile uint16_t *)0xFF8240)
#define VID_SHIFT_MODE_ST (*(volatile uint8_t *)0xFF8260)
#define VID_SHIFT_MODE_TT (*(volatile uint8_t *)0xFF8262)
#define VID_HSCROLL_STE (*(volatile uint8_t *)0xFF8265)

#define DISK_BASE_H (*(volatile uint8_t *)0xFF8609)
#define DISK_BASE_M (*(volatile uint8_t *)0xFF860B)
#define DISK_BASE_L (*(volatile uint8_t *)0xFF860D)
#define DISK_DAT (*(volatile uint16_t *)0xFF8604)
#define DISK_CTL (*(volatile uint16_t *)0xFF8606)
#define DISK_STAT_R (*(volatile uint16_t *)0xFF8606)

#define PSG_REG (*(volatile uint8_t *)0xFF8800)
#define PSG_DAT_R (*(volatile uint8_t *)0xFF8800)
#define PSG_DAT_W (*(volatile uint8_t *)0xFF8802)


