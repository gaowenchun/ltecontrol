
//=================================================================================
//Name         :wellan.cpp
//Auther	   :ict
//version	   :
//Description  :
//=================================================================================

#include <iostream>
#include "includes/Configure.h"
#include "includes/Manager.h"
#include "includes/DownManager.h"
#include "includes/IMSITable.h"
#include "includes/log.h"
#define IMSI_dB "../Database/IMSI_TABLE.db"
using namespace std;

sock_fd msock_fd;
IMSITable gIMSITable;

int main()
{
    signal(SIGPIPE,SIG_IGN);
    pthread_t tuplink_read;
    pthread_t tuplink_handle;
    pthread_t tdlink_handle;
    
    // Init the log4cplus and set parameters
    InitLogger();
    
    // manipulate configuration database files
    Control_Config();
    
    // create the IMSI_Table Database
    gIMSITable.open(IMSI_dB);
    
    // create link tcp and udp
    ControlManager Manger(udp_port,udp_addr,tcp_port,tcp_addr);
    msock_fd.tcp_fd = Manger.init_tcp();
    msock_fd.udp_fd = Manger.init_udp();
    
    // thread down link
    pthread_create(&tdlink_handle,NULL,(void*(*)(void*))thread_down,(void*)&msock_fd);
    
    //thread up link write FIFO
    pthread_create(&tuplink_read, NULL,(void*(*)(void*))thread_up,(void*)&msock_fd);
    sleep(1);
    
    //thread up link read FIFO and handle
    pthread_create(&tuplink_handle, NULL,(void*(*)(void*))pthread_handle_up_message,(void*)&msock_fd);

    pthread_join(tdlink_handle,NULL);
    pthread_join(tuplink_read,NULL);
    pthread_join(tuplink_handle,NULL);
    
    return 0;
}

