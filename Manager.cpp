//======================================================================
//Name:
//Data:			2017-1-10
//Author:
//Descriptor:		
//=======================================================================

#include "includes/Manager.h"
#include "includes/Configure.h"
#include "includes/IMSITable.h"
#include "includes/DownManager.h"
#include "includes/log.h"
#include <ctime>
#define control_id "LMC00001111"
#define MAXEPOLLSIZE 100
										
extern int errno;
extern IMSITable gIMSITable;
bool tcp_fd_flag = 0;
TransmitFIFO gTransmitFIFO;
struct InterthreadQueue* mTransmitFIFO;
pthread_t threadimsi;
int tcp_fd;

int Select_Call(void *data,int col_count,char **col_values,char **col_name)
{
    char imsibuf[256];
    char sendIMSI[256];
   
    sprintf(imsibuf,"#%s#%s#%s#%s#%s",control_id,col_values[3]==0?"NULL":col_values[3],col_values[2]==0?"NULL":col_values[2],\
		col_values[1]==0?"NULL":col_values[1],col_values[5]==0?"NULL":col_values[5]);
   
   ((uint16_t*)sendIMSI)[0] = htons(strlen(imsibuf));
    memcpy((sendIMSI+2),imsibuf,strlen(imsibuf));
   
    if(send(tcp_fd, sendIMSI, strlen(imsibuf)+2, 0) == -1)
      
      LOG_ERROR("IMSI RESEND ERROR \n");
    else
      LOG_WARN("IMSI SEND SUCCESS \n");
   
    return 0;
}

void IMSIHandler()
{   
    char *zErrMsg = 0;
    char cmd1[256];
    sqlite3 *db = 0;
    sleep(10);
    
    int ret = sqlite3_open("../Database/IMSI_TABLE.db",&db);
    if(ret != SQLITE_OK)
    {
        LOG_ERROR("OPEN SQLITE ERROR: %s "<< sqlite3_errmsg(db));
        return;
    }
    
    sprintf(cmd1,"SELECT * FROM IMSI_TABLE WHERE Access = '0';"); 
    sqlite3_exec(db, cmd1, Select_Call, 0, &zErrMsg);
    
    sprintf(cmd1,"UPDATE IMSI_TABLE SET Access = '1' WHERE Access = '2';");
    sqlite3_exec(db, cmd1, 0, 0, &zErrMsg);
    
    sqlite3_close(db);
}

void InitFIFO()
{
    mTransmitFIFO = gTransmitFIFO.QueueOpen();
    if(!mTransmitFIFO)
    {
        LOG_ERROR("open FIFO failure");
        gTransmitFIFO.QueueClose(mTransmitFIFO);
    }
}

void tcp_data_packing(unsigned char* Send_Buf,char* buf)
{
    ((uint16_t*)Send_Buf)[0] = htons(strlen(buf));   //2 BYTE Fill senddata length
    memcpy((Send_Buf+2),buf,strlen(buf));
    LOG_INFO((int)strlen(buf)<<" "<<(char*)(Send_Buf+2));
}

/*
   pthread_handle_message 
*/
void pthread_handle_up_message(int* msock_fd)
{
    int udp_fd = msock_fd[1];
    tcp_fd = msock_fd[0];
    unsigned char* writeBuffer;
    char buf[2048];
    char sendhead[128];
    struct dataSocket* recvbuf;
    time_t start, tmp;
    time(&start);	
    bool Flag = 1;
    int sendret;
    while(1)
    {
      time(&tmp);
      if(difftime(tmp,start) >= 10 && Flag == 1)
      {
	startcell(udp_fd,"192.168.1.90");
	LOG_WARN("DONT RECV HEART TEN SECONDS");
	Flag = 0;
      }
      
      writeBuffer = (unsigned char*)readNoBlock(mTransmitFIFO);
      unsigned char Send_Buf[2048] = {'\0'};
      
      if(writeBuffer)
      {
	recvbuf = (struct dataSocket*)writeBuffer;
	responsehead responsehead(recvbuf->msgdata);
	
	if(strcmp("101",responsehead.getmsgtype()) == 0)
	{
	  time(&start);
	  Flag = 1;
	  Msg_Heart responseheart;
	  responseheart.bufferparse((char*)recvbuf->msgdata); 
	  sprintf(sendhead,"#%s#%s#%s",control_id,responseheart.railname,responseheart.cardid);
	  sprintf(buf,"%s#%s\n",sendhead,recvbuf->msgdata);
	  tcp_data_packing(Send_Buf,buf); 
	  sendret = send(tcp_fd, Send_Buf, strlen(buf)+2, 0);
	  
	}else if(strcmp("103",responsehead.getmsgtype()) == 0){
	  
	  Msg_Imsi* responseimsi = new Msg_Imsi();
   	  responseimsi->bufferparse((char*)recvbuf->msgdata);  

	  sprintf(sendhead,"#%s#%s#%s",control_id,responseimsi->railname,responseimsi->cardid);
   	  sprintf(buf,"%s#%s\n",sendhead,recvbuf->msgdata);
	  tcp_data_packing(Send_Buf,buf); 
	  sendret = send(tcp_fd, Send_Buf, strlen(buf)+2, 0);
	  
	  if(sendret == -1)
	  {
	    tcp_fd_flag = 0;
	  }
	  LOG_DEBUG("THE FLAG OF SEVSES WHEATER RUN SUCCESS,IF SUCCESS IS 1 " << tcp_fd_flag);  
	  gIMSITable.assign(responseimsi,tcp_fd_flag);
	  
	  delete responseimsi;
	  
	}else{

	  sprintf(sendhead,"#%s#%s#%s",control_id,"0","0");
   	  sprintf(buf,"%s#%s\n",sendhead,recvbuf->msgdata);
	  tcp_data_packing(Send_Buf,buf);
	  sendret = send(tcp_fd, Send_Buf, strlen(buf)+2, 0);
	}

	if(sendret!= -1){
	  
	  tcp_fd_flag = 1;
	  
	}else{
	  
	  if(errno != 0 && tcp_fd_flag == 1)
	  {
	    close(tcp_fd);
	    tcp_fd_flag = 0;
	  }
	  
   	  ControlManager Manger_repeat(udp_port,udp_addr,tcp_port,tcp_addr);
          tcp_fd = Manger_repeat.init_tcp();
	  LOG_WARN("Connect tcp_fd is " << tcp_fd);
	 
	  if(tcp_fd != -1)
	  {
	    if(pthread_create(&threadimsi, NULL, (void*(*)(void*))IMSIHandler, NULL))
	      LOG_ERROR("Create resend imsi pthread error!");
	  }  
      }
	
      free(recvbuf->msgdata);
      free(recvbuf);      
      }   
    }  
    pthread_join(threadimsi,NULL); 
}

void POll_write(int* msock_fd)
{
    char buffer[MAXBUF];
    int  udp_fd = msock_fd[1];
    struct sockaddr_in client_addr;
    socklen_t cli_len = sizeof(client_addr);
    char* msg_data;
    unsigned int msglen = 0;
    dataSocket* datasoc;
    
    msglen = recvfrom(udp_fd, buffer, MAXBUF, 0, (struct sockaddr *)&client_addr, &cli_len);
    if(msglen < 0 || msglen> 4096)
    {
      printf("data socket read error\n");
      return;
    }
    datasoc = (dataSocket*)malloc(sizeof(dataSocket));
    if(!datasoc)
    {
      printf("NO Memory for data socket\n");
    }
    datasoc->size = msglen;
    msg_data = (char*)malloc(msglen+1);
    if(!msg_data)
    {
	LOG_ERROR("NO Memory for msg_data socket\n");
    }
    memset(msg_data,'\0',msglen+1);
    memcpy(msg_data,buffer,msglen);
    
    datasoc->msgdata = msg_data;
    Write(mTransmitFIFO,datasoc);
}

int thread_up(int *msock_fd)
{
    int kdpfd, nfds, n, udp_fd, tcp_fd;
    udp_fd = msock_fd[1];
    tcp_fd = msock_fd[0];
    LOG_INFO("TCP_FD IS"<< tcp_fd<<" UDP_FD IS "<< udp_fd);
    
    InitFIFO();
    struct epoll_event ev;
    struct epoll_event events[MAXEPOLLSIZE];
    kdpfd = epoll_create(MAXEPOLLSIZE);
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = udp_fd;

    if (epoll_ctl(kdpfd, EPOLL_CTL_ADD, udp_fd, &ev) < 0)
    {
        LOG_ERROR("epoll set insertion error "<< udp_fd);
        return -1;
    }
    else
    {
        LOG_INFO("socket  epoll success\n");
    }
    while (1)
    {
        nfds = epoll_wait(kdpfd, events, 1000, -1);
        if (nfds == -1)
        {
            LOG_ERROR("epoll_wait");
            break;
        }
        for (n = 0; n < nfds; ++n)
        {
            if (events[n].data.fd == udp_fd)
            {
		POll_write(msock_fd);
            }
        }
    }
    close(udp_fd);
}
