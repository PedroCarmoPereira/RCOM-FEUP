#ifndef ERROR_INJECTOR
#define ERROR_INJECTOR

#define FH_ER   0 /*10% chance to inject an error in the frame header*/
#define DATA_ER 10 /*10% chance to inject an error in the data field*/

void inject_data_bcc(char * data, int size);

void inject_frame(char * frame);

#endif
