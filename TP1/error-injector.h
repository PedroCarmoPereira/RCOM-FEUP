#ifndef ERROR_INJECTOR
#define ERROR_INJECTOR

//#define FH_ER   20 /*10% chance to inject an error in the frame header*/ SE TIVERMOS UM ERRO AQUI ELE NUNCA SAIDAS SM'S...
#define DATA_ER 10 /*10% chance to inject an error in the data field*/

void inject_data_bcc(char * data, int size);

void inject_frame_bcc(char * frame);

#endif
