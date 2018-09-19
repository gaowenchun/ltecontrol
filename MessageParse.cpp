//=====================================================================
//Name              :MessageParse.cpp
//Author            :ict
//Version           :
//Description       :
//=====================================================================

#include<iostream>
#include<assert.h>
#include<cstring>
#include "includes/MessageParse.h"

using namespace std;
char* varcpy(char* temp,char* tep,char* str,int &j)
{
   int i = 0, k = 0, flag = 0;
   while(str[j]!='\0' && str[j]!= ' ')
   {
	temp[i] = str[j];
	if(temp[i] == '[')
	{
	    flag = 1;
	    while(str[j]!=']')
	    {
		tep[k] = str[++j];
 		k++;
	    }
	    tep[--k] = '\0';
	}       
	i++;
	j++;	
   }
   temp[i] = '\0';
   if(str[j] ==' ')
   j++;
   if(flag == 1){
        return tep;
   }else
   {
	return temp;
   }
}

void constructbuffer(char* buffer)
{
    memset(buffer,0,sizeof(buffer));
}

responsehead::responsehead(char* wstart)
{
    memset(startflag,'\0',sizeof(startflag));
    memcpy(startflag,wstart,3);
}

void Msg_Imsi::bufferparse(char* getbuffer)
{
    assert(getbuffer);
    int j = 0;
    char tmp[124] = {'\0'};
    char tep[124] = {'\0'};
    while(getbuffer[j]!='\0')
    {
        strcpy(startflag,varcpy(tmp,tep,getbuffer,j));
        strcpy(msgname,varcpy(tmp,tep,getbuffer,j));
        strcpy(length,varcpy(tmp,tep,getbuffer,j));
        strcpy(railname,varcpy(tmp,tep,getbuffer,j));
        strcpy(cardid,varcpy(tmp,tep,getbuffer,j));
        strcpy(ID,varcpy(tmp,tep,getbuffer,j));
        strcpy(time,varcpy(tmp,tep,getbuffer,j));
        strcpy(taType,varcpy(tmp,tep,getbuffer,j));
        strcpy(rsrp,varcpy(tmp,tep,getbuffer,j));
        strcpy(ulCqi,varcpy(tmp,tep,getbuffer,j));
        strcpy(ulRssi,varcpy(tmp,tep,getbuffer,j));
        strcpy(imsi,varcpy(tmp,tep,getbuffer,j));
        strcpy(imei,varcpy(tmp,tep,getbuffer,j));
   }
}

void ErrorIndi::bufferparse(char* getbuffer)
{
    assert(getbuffer);
}

void Msg_Heart::bufferparse(char* getbuffer)
{
    assert(getbuffer);
    int j = 0;
    char tmp[124] = {'\0'};
    char tep[124] = {'\0'};
    while(getbuffer[j]!='\0')
    {
        strcpy(startflag,varcpy(tmp,tep,getbuffer,j));
        strcpy(msgname,varcpy(tmp,tep,getbuffer,j));
        strcpy(length,varcpy(tmp,tep,getbuffer,j));
        strcpy(railname,varcpy(tmp,tep,getbuffer,j));
        strcpy(cardid,varcpy(tmp,tep,getbuffer,j));
        strcpy(time,varcpy(tmp,tep,getbuffer,j));
        strcpy(version,varcpy(tmp,tep,getbuffer,j));
        strcpy(build_date,varcpy(tmp,tep,getbuffer,j));
        strcpy(tmp,varcpy(tmp,tep,getbuffer,j));
        strcpy(gps,varcpy(tmp,tep,getbuffer,j));
        strcpy(status,varcpy(tmp,tep,getbuffer,j));
   }
}

