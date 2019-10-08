#ifndef MACROS_H
#define MACROS_H

#define SFD 		0x7e
#define SET 		0x03
#define SUP_SIZE	5
#define UA 			0x07
#define CE_RR 		0x03
#define	CR_RE 		0x01
#define TRIES		3

enum state {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_RCV,
    END
} state;

#endif
