//=====================================================================
//Name              :DownManager.cpp
//Author            :ict
//Version           :
//Description       :
//=====================================================================

#include "includes/DownManager.h"

void startcell(int ufd, char* IP)
{
    char startbuf[BUFFERLEN] = "StartCell";
    struct sockaddr_in address;//处理网络通信的地址 
    
    bzero(&address,sizeof(address));
    address.sin_family=AF_INET;
    address.sin_addr.s_addr=inet_addr(IP);//这里不一样  
    address.sin_port=htons(9001);
    
    int len = sendto(ufd,startbuf,strlen(startbuf),0,(struct sockaddr *)&address,sizeof(address));
    if(len < 0)
    {
	LOG_ERROR("SEND TO LTE ENODEB START MESSAGE IS ERROR" << IP);
    }
}

int parsedownMessage(char* recvbuf,char* sendbuf,char* sendip)
{

    int len = htons(((unsigned short *)recvbuf)[0]);
    
    int i, k, j = 0;
    char (*buf)[20] = (char(*)[20])malloc(sizeof(char)*1000);

    for(i = 3; i < len; i++)
    {
        k = 0;
        while(recvbuf[i] != '#' && recvbuf[i] != '\0')
        {
            buf[j][k] = recvbuf[i];
            k++;
            i++;
        }
        buf[j][k] = '\0';
        printf("%s\n",buf[j]);
        j++;
    }
    
    memcpy(sendbuf,buf[j-1],strlen(buf[j-1]));
    
    strcpy(sendip, CUCCIP);
    printf("sendbuf is %s\n",sendbuf);
    printf("sendIP is %s\n",sendip); 
    
    free(buf);
}

void down_send(int ufd,struct sockaddr_in &address,char* IP,char* sendbuf)
{
    address.sin_family=AF_INET;
    address.sin_addr.s_addr=inet_addr(IP);//这里不一样  
    address.sin_port=htons(9001);
    int len = sendto(ufd,sendbuf,strlen(sendbuf),0,(struct sockaddr *)&address,sizeof(address));
    if(len < 0)
    {
        LOG_ERROR("send start message %s is error \n" << IP);
    }
}


int thread_down(int* msock_fd)
{
    int tfd, ufd, len,i,nCount;
    tfd = msock_fd[0];
    ufd = msock_fd[1];
    char recvbuf[BUFFERLEN];
    char sendbuf[BUFFERLEN];
    char sendip[BUFFERLEN];
    
    struct sockaddr_in address;//处理网络通信的地址  
    bzero(&address,sizeof(address));
    
    startcell(ufd, CMCCIP);
    startcell(ufd, CUCCIP);
    startcell(ufd, CTCCIP);
    
    while(1)
    {
        memset(recvbuf,'\0',BUFFERLEN);
        len = recv(tfd, recvbuf, BUFFERLEN, 0);
        if(len > 0)
        {
	   parsedownMessage(recvbuf,sendbuf,sendip);   
	   down_send(ufd,address,sendip,sendbuf);
        }
    }
    
    return 0;
}

