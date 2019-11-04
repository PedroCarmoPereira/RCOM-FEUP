#include <stdio.h>
#include <stdlib.h>
#include "error-injector.h"

int seeded = 0;

static inline void negateByte(char * byte){
    *byte = ~(*byte);
}

void inject_data_bcc(char * data, int size){
    if(!seeded) {
        srand(time(NULL));
        seeded = 1;
    }
    unsigned int random_no = rand() % 100 + 1;
    if(random_no <= DATA_ER) {
        puts("PACKET INJECTED");
        negateByte(&data[size - 1]);
    }
}

void inject_frame(char * frame){
    if(!seeded) {
        srand(time(NULL));
        seeded = 1;
    }
    unsigned int random_no = rand() % 100 + 1;
    if(random_no <= FH_ER){
        puts("FRAME INJECTED");
        frame[3] = 0x7e;
    }
}