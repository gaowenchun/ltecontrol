//=====================================================================
//Name              :Configure.cpp
//Author            :ict
//Version           :
//Description       :
//=====================================================================

#include "includes/Configure.h"
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <pthread.h>
#include <assert.h>
#include "includes/sqlite3.h"
#include "includes/log.h"

#define MAXSIZE 100

int tcp_port;
int udp_port;
char tcp_addr[256];
char udp_addr[256];

int Control_Config()
{
    sqlite3* db = NULL;
    char* msg = NULL;
    int  nRow, nCol;
    char** pResult;
  
    if(sqlite3_open(Control_dB,&db))
    {
       LOG_ERROR("Configure Open is error \n");
       
    }else
    {
       char* sql = "CREATE TABLE IF NOT EXISTS CONFIG(\
                KEYSTRING TEXT UNIQUE NOT NULL,\
                VALUESTRING TEXT,\
                STATIC INTEGER DEFAULT 0,\ 
                OPTIONAL INTEGER DEFAULT 0,\
                COMMENTS TEXT DEFAULT ''\
     )";
     
       sqlite3_exec(db, sql, 0, 0, &msg);
       sql = "select * from CONFIG;";
       
       if(sqlite3_get_table(db,sql,&pResult,&nRow,&nCol,NULL) == SQLITE_OK)
       {
         tcp_port = atoi(pResult[6]);
         udp_port = atoi(pResult[11]);
         memcpy(tcp_addr,pResult[16],strlen(pResult[16]));
         memcpy(udp_addr,pResult[21],strlen(pResult[21]));
         sqlite3_free_table(pResult);
       }
    }  
    sqlite3_close(db);
}

int ControlManager::setnonblocking(int sockfd)
{
    if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)|O_NONBLOCK) == -1)
    {
        return -1;
    }
    return 0;
}

int ControlManager::init_udp()
{
    int listener, kdpfd, nfds, n, curfds;
    socklen_t len;
    struct sockaddr_in local_addr;
    struct rlimit rt;
    rt.rlim_max = rt.rlim_cur = MAXSIZE;
    if (setrlimit(RLIMIT_NOFILE, &rt) == -1)
    {
        LOG_ERROR("setrlimit");
        exit(1);
    }
    if ((listener = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
    {
        LOG_ERROR("socket listener is error\n");
        exit(1);
    }
    int opt=SO_REUSEADDR;
    setsockopt(listener,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    setnonblocking(listener);
    bzero(&local_addr, sizeof(local_addr));
    
    local_addr.sin_family = PF_INET;
    local_addr.sin_port = htons(UDPPORT);
    local_addr.sin_addr.s_addr = inet_addr(udp_address);
    
    if (bind(listener, (struct sockaddr *) &local_addr, sizeof(struct sockaddr)) == -1)
    {
        LOG_ERROR("UDP IP bind FAILED");
        exit(1);
    }
    else
    {
        LOG_INFO("UDP IP bind success");
    }
    return listener;
}

int ControlManager::init_tcp()
{
    int connect_fd, ret, i, len;
    static struct sockaddr_in srv_addr;
    
    connect_fd=socket(PF_INET,SOCK_STREAM,0);
    if(connect_fd < 0)
    {
        LOG_ERROR("cannot create communication socket");
        return 1;
    }
    memset(&srv_addr,0,sizeof(srv_addr));
    
    srv_addr.sin_family=AF_INET;
    srv_addr.sin_addr.s_addr=inet_addr(tcp_address);
    srv_addr.sin_port=htons(TCPPORT);
    
    ret = connect(connect_fd,(struct sockaddr*)&srv_addr,sizeof(srv_addr));
    if(ret==-1){
        LOG_ERROR("init  cannot connect to the server");
        close(connect_fd);
        return -1;
    }else{
	LOG_INFO("connect first server success\n");
    }
    return connect_fd;
}



