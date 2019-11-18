#include <stdlib.h>
#include <stdio.h>
#include <netdb.h> 
#include <sys/types.h>
#include <netinet/in.h> 
#include <arpa/inet.h>

#include "rfc1738url.h"
#include "ftp.h"


int getHostInfo(rfc1738url * url){
    
    host = gethostbyname(url->hostname);
    
    if(host == NULL){
        printf("could not resolver hostame:%s", url->hostname);
        return -1;
    }

    return 0;
}
