#include "mmio.h"

#if (defined(MMIO_IMPL) && !defined(SH4MMIO_IMPL)) || \
    (!defined(MMIO_IMPL) && !defined(SH4MMIO_IFACE))

#ifdef MMIO_IMPL
#define SH4MMIO_IMPL
#else
#define SH4MMIO_IFACE
#endif
/* SH7750 onchip mmio devices */

MMIO_REGION_BEGIN( 0xFF000000, MMU, "MMU Registers" )
    LONG_PORT( 0x000, PTEH, PORT_MRW, UNDEFINED, "Page table entry high" )
    LONG_PORT( 0x004, PTEL, PORT_MRW, UNDEFINED, "Page table entry low" )
    LONG_PORT( 0x008, TTB,  PORT_MRW, UNDEFINED, "Translation table base" )
    LONG_PORT( 0x00C, TEA,  PORT_MRW, UNDEFINED, "TLB exception address" )
    LONG_PORT( 0x010, MMUCR,PORT_MRW, 0, "MMU control register" )
    BYTE_PORT( 0x14, BASRA, PORT_MRW, UNDEFINED, "Break ASID A" ) /* UBC */
    BYTE_PORT( 0x18, BASRB, PORT_MRW, UNDEFINED, "Break ASID B" ) /* UBC */
    LONG_PORT( 0x01C, CCR,  PORT_MRW, 0, "Cache control register" )
    LONG_PORT( 0x020, TRA,  PORT_MRW, UNDEFINED, "TRAPA exception register" )
    LONG_PORT( 0x024, EXPEVT,PORT_MRW, 0, "Exception event register" )
    LONG_PORT( 0x028, INTEVT,PORT_MRW, UNDEFINED, "Interrupt event register" )
    LONG_PORT( 0x034, PTEA, PORT_MRW, UNDEFINED, "Page table entry assistance" )
    LONG_PORT( 0x038, QACR0,PORT_MRW, UNDEFINED, "Queue address control 0" )
    LONG_PORT( 0x03C, QACR1,PORT_MRW, UNDEFINED, "Queue address control 1" )
MMIO_REGION_END

/* User Break Controller (Page 717 [757] of sh7750h manual) */
MMIO_REGION_BEGIN( 0xFF200000, UBC, "User Break Controller" )
    LONG_PORT( 0x000, BARA, PORT_MRW, UNDEFINED, "Break address A" )
    BYTE_PORT( 0x004, BAMRA, PORT_MRW, UNDEFINED, "Break address mask A" )
    WORD_PORT( 0x008, BBRA, PORT_MRW, 0, "Break bus cycle A" )
    LONG_PORT( 0x00C, BARB, PORT_MRW, UNDEFINED, "Break address B" )
    BYTE_PORT( 0x010, BAMRB, PORT_MRW, UNDEFINED, "Break address mask B" )
    WORD_PORT( 0x014, BBRB, PORT_MRW, 0, "Break bus cycle B" )
    LONG_PORT( 0x018, BDRB, PORT_MRW, UNDEFINED, "Break data B" )
    LONG_PORT( 0x01C, BDMRB, PORT_MRW, UNDEFINED, "Break data mask B" )
    WORD_PORT( 0x020, BRCR, PORT_MRW, 0, "Break control" )
MMIO_REGION_END
/* Bus State Controller (Page 293 [333] of sh7750h manual)
 * I/O Ports */
MMIO_REGION_BEGIN( 0xFF800000, BSC, "Bus State Controller" )
    LONG_PORT( 0x000, BCR1, PORT_MRW, 0, "Bus control 1" )
    WORD_PORT( 0x004, BCR2, PORT_MRW, 0x3FFC, "Bus control 2" )
    LONG_PORT( 0x008, WCR1, PORT_MRW, 0x77777777, "Wait state control 1" )
    LONG_PORT( 0x00C, WCR2, PORT_MRW, 0xFFFEEFFF, "Wait state control 2" )
    LONG_PORT( 0x010, WCR3, PORT_MRW, 0x07777777, "Wait state control 3" )
    LONG_PORT( 0x014, MCR, PORT_MRW, 0, "Memory control register" )
    WORD_PORT( 0x018, PCR, PORT_MRW, 0, "PCMCIA control register" )
    WORD_PORT( 0x01C, RTCSR, PORT_MRW, 0, "Refresh timer control/status" )
    WORD_PORT( 0x020, RTCNT, PORT_MRW, 0, "Refresh timer counter" )
    WORD_PORT( 0x024, RTCOR, PORT_MRW, 0, "Refresh timer constant" )
    WORD_PORT( 0x028, RFCR, PORT_MRW, 0, "Refresh count" )
    LONG_PORT( 0x02C, PCTRA, PORT_MRW, 0, "Port control register A" )
    WORD_PORT( 0x030, PDTRA, PORT_RW, UNDEFINED, "Port data register A" )
    LONG_PORT( 0x040, PCTRB, PORT_MRW, 0, "Port control register B" )
    WORD_PORT( 0x044, PDTRB, PORT_RW, UNDEFINED, "Port data register B" )
    WORD_PORT( 0x048, GPIOIC, PORT_MRW, 0, "GPIO interrupt control register" )
MMIO_REGION_END

/* DMA Controller (Page 457 [497] of sh7750h manual) */
MMIO_REGION_BEGIN( 0xFFA00000, DMAC, "DMA Controller" )
    LONG_PORT( 0x000, SAR0, PORT_MRW, UNDEFINED, "DMA source address 0" )
    LONG_PORT( 0x004, DAR0, PORT_MRW, UNDEFINED, "DMA destination address 0" )
    LONG_PORT( 0x008, DMATCR0, PORT_MRW, UNDEFINED, "DMA transfer count 0" )
    LONG_PORT( 0x00C, CHCR0, PORT_MRW, 0, "DMA channel control 0" )
    LONG_PORT( 0x010, SAR1, PORT_MRW, UNDEFINED, "DMA source address 1" )
    LONG_PORT( 0x014, DAR1, PORT_MRW, UNDEFINED, "DMA destination address 1" )
    LONG_PORT( 0x018, DMATCR1, PORT_MRW, UNDEFINED, "DMA transfer count 1" )
    LONG_PORT( 0x01C, CHCR1, PORT_MRW, 0, "DMA channel control 1" )
    LONG_PORT( 0x020, SAR2, PORT_MRW, UNDEFINED, "DMA source address 2" )
    LONG_PORT( 0x024, DAR2, PORT_MRW, UNDEFINED, "DMA destination address 2" )
    LONG_PORT( 0x028, DMATCR2, PORT_MRW, UNDEFINED, "DMA transfer count 2" )
    LONG_PORT( 0x02C, CHCR2, PORT_MRW, 0, "DMA channel control 2" )
    LONG_PORT( 0x030, SAR3, PORT_MRW, UNDEFINED, "DMA source address 3" )
    LONG_PORT( 0x034, DAR3, PORT_MRW, UNDEFINED, "DMA destination address 3" )
    LONG_PORT( 0x038, DMATCR3, PORT_MRW, UNDEFINED, "DMA transfer count 3" )
    LONG_PORT( 0x03C, CHCR3, PORT_MRW, 0, "DMA channel control 3" )
    LONG_PORT( 0x040, DMAOR, PORT_MRW, 0, "DMA operation register" )
MMIO_REGION_END

/* Clock Pulse Generator (page 233 [273] of sh7750h manual) */
MMIO_REGION_BEGIN( 0xFFC00000, CPG, "Clock Pulse Generator" )
    WORD_PORT( 0x000, FRQCR, PORT_MRW, UNDEFINED, "Frequency control" )
    BYTE_PORT( 0x004, STBCR, PORT_MRW, 0, "Standby control" )
    BYTE_PORT( 0x008, WTCNT, PORT_MRW, 0, "Watchdog timer counter" )
    BYTE_PORT( 0x00C, WTCSR, PORT_MRW, 0, "Watchdog timer control/status" )
    BYTE_PORT( 0x010, STBCR2, PORT_MRW, 0, "Standby control 2" )
MMIO_REGION_END

/* Real time clock (Page 253 [293] of sh7750h manual) */
MMIO_REGION_BEGIN( 0xFFC80000, RTC, "Realtime Clock" )
    BYTE_PORT( 0x000, R64CNT, PORT_R, UNDEFINED, "64 Hz counter" )
    BYTE_PORT( 0x004, RSECCNT, PORT_MRW, UNDEFINED, "Second counter" )
    /* ... */
MMIO_REGION_END

/* Interrupt controller (Page 699 [739] of sh7750h manual) */
MMIO_REGION_BEGIN( 0xFFD00000, INTC, "Interrupt Controller" )
    WORD_PORT( 0x000, ICR, PORT_MRW, 0x0000, "Interrupt control register" )
    WORD_PORT( 0x004, IPRA, PORT_MRW, 0x0000, "Interrupt priority register A" )
    WORD_PORT( 0x008, IPRB, PORT_MRW, 0x0000, "Interrupt priority register B" )
    WORD_PORT( 0x00C, IPRC, PORT_MRW, 0x0000, "Interrupt priority register C" )
    WORD_PORT( 0x010, IPRD, PORT_MRW, 0xDA74, "Interrupt priority register D" )
MMIO_REGION_END

/* Timer unit (Page 277 [317] of sh7750h manual) */
MMIO_REGION_BEGIN( 0xFFD80000, TMU, "Timer Unit" )
    BYTE_PORT( 0x000, TOCR, PORT_MRW, 0x00, "Timer output control register" )
    BYTE_PORT( 0x004, TSTR, PORT_MRW, 0x00, "Timer start register" )
    LONG_PORT( 0x008, TCOR0, PORT_MRW, 0xFFFFFFFF, "Timer constant 0" )
    LONG_PORT( 0x00C, TCNT0, PORT_MRW, 0xFFFFFFFF, "Timer counter 0" )
    WORD_PORT( 0x010, TCR0, PORT_MRW, 0x0000, "Timer control 0" )
    LONG_PORT( 0x014, TCOR1, PORT_MRW, 0xFFFFFFFF, "Timer constant 1" )
    LONG_PORT( 0x018, TCNT1, PORT_MRW, 0xFFFFFFFF, "Timer counter 1" )
    WORD_PORT( 0x01C, TCR1, PORT_MRW, 0x0000, "Timer control 1" )
    LONG_PORT( 0x020, TCOR2, PORT_MRW, 0xFFFFFFFF, "Timer constant 2" )
    LONG_PORT( 0x024, TCNT2, PORT_MRW, 0xFFFFFFFF, "Timer counter 2" )
    WORD_PORT( 0x028, TCR2, PORT_MRW, 0x0000, "Timer control 2" )
    LONG_PORT( 0x02C, TCPR2, PORT_R, UNDEFINED, "Input capture register" )
MMIO_REGION_END

/* Serial channel (page 541 [581] of sh7750h manual) */
MMIO_REGION_BEGIN( 0xFFE00000, SCI, "Serial Communication Interface" )
    BYTE_PORT( 0x000, SCSMR1, PORT_MRW, 0x00, "Serial mode register" )
    BYTE_PORT( 0x004, SCBRR1, PORT_MRW, 0xFF, "Bit rate register" )
    BYTE_PORT( 0x008, SCSCR1, PORT_MRW, 0x00, "Serial control register" )
    BYTE_PORT( 0x00C, SCTDR1, PORT_MRW, 0xFF, "Transmit data register" )
    BYTE_PORT( 0x010, SCSSR1, PORT_MRW, 0x84, "Serial status register" )
    BYTE_PORT( 0x014, SCRDR1, PORT_R, 0x00, "Receive data register" )
    BYTE_PORT( 0x01C, SCSPTR1, PORT_MRW, 0x00, "Serial port register" )
MMIO_REGION_END

MMIO_REGION_BEGIN( 0xFFE80000, SCIF, "Serial Controller (FIFO) Registers" )
    WORD_PORT( 0x000, SCSMR2, PORT_MRW, 0x0000, "Serial mode register (FIFO)" )
MMIO_REGION_END

MMIO_REGION_LIST_BEGIN( sh4mmio )
    MMIO_REGION( MMU )
    MMIO_REGION( UBC )
    MMIO_REGION( BSC )
    MMIO_REGION( DMAC )
    MMIO_REGION( CPG )
    MMIO_REGION( RTC )
    MMIO_REGION( INTC )
    MMIO_REGION( TMU )
    MMIO_REGION( SCI )
    MMIO_REGION( SCIF )
MMIO_REGION_LIST_END

#endif
