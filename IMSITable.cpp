//=====================================================================
//Name				:IMSITable.cpp
//Author			:ict
//Version			:
//Description		:
//=====================================================================

#include "includes/IMSITable.h"
#include "includes/sqlite3.h"
#include "includes/log.h"

static const char* createIMSITable = {
    "CREATE TABLE IF NOT EXISTS IMSI_TABLE("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT,"//ID:PRIMARY KEY
        "IMSI TEXT NOT NULL,"//IMSI
        "HARDWARE TEXT NOT NULL,"//HARDWARE:Hardware name
        "WlName TEXT NOT NULL,"//WlName: WeiLan name
        "SerialNum TEXT NOT NULL,"//SerialNum: IMSI serial number
        "Time TEXT NOT NULL,"//Time: The hardware running time
        "Rsrq TEXT,"//Rsrq:
        "UlCqi TEXT,"//UlCqi:
        "UlRssi TEXT,"//UlRssi:
	"Access INTEGER"
    ")"
};

int sqlite3_prepare_statement(sqlite3* DB, sqlite3_stmt **stmt, const char* query)
{
    int prc = sqlite3_prepare_v2(DB,query,strlen(query),stmt,NULL);
    if (prc) {
      LOG_ERROR("sqlite3_prepare_v2 failed for \"%s\": %s\n"<<query << " " << sqlite3_errmsg(DB));
      sqlite3_finalize(*stmt);
    }
    return prc;      
}

int sqlite3_run_query(sqlite3* DB, sqlite3_stmt *stmt)
{
    int src = SQLITE_BUSY;
    while (src==SQLITE_BUSY) {
      src = sqlite3_step(stmt);
      if (src==SQLITE_BUSY) {
	usleep(100000);
      }
    }
    if ((src!=SQLITE_DONE) && (src!=SQLITE_ROW)) {
      LOG_ERROR("sqlite3_run_query failed: %s: %s\n"<< sqlite3_sql(stmt)<< " "<<sqlite3_errmsg(DB));
    }
    return src;   
}

bool sqlite3_exists(sqlite3* DB, const char *tableName,
                const char* keyName, const char* keyData)
{
    size_t stringSize = 100 + strlen(tableName) + strlen(keyName) + strlen(keyData);
    char query[stringSize];
    sprintf(query,"SELECT * FROM %s WHERE %s == \"%s\"",tableName,keyName,keyData);
    // Prepare the statement.
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_statement(DB,&stmt,query)) return false;
    // Read the result.
    int src = sqlite3_run_query(DB,stmt);
    sqlite3_finalize(stmt);
    // Anything there?
    return (src == SQLITE_ROW);
}

// This function returns an allocated string that must be free'd by the caller.
bool sqlite3_single_lookup(sqlite3* DB, const char* tableName,
                const char* keyName, const char* keyData,
                const char* valueName, char* &valueData)
{
    valueData=NULL;
    size_t stringSize = 100 + strlen(valueName) + strlen(tableName) + strlen(keyName) + strlen(keyData);
    char query[stringSize];
    sprintf(query,"SELECT %s FROM %s WHERE %s == \"%s\"",valueName,tableName,keyName,keyData);
    // Prepare the statement.
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_statement(DB,&stmt,query)) return false;
    // Read the result.
    int src = sqlite3_run_query(DB,stmt);
    bool retVal = false;
    if (src == SQLITE_ROW) {
        const char* ptr = (const char*)sqlite3_column_text(stmt,0);
        if (ptr) valueData = strdup(ptr);
        retVal = true;
    }
    sqlite3_finalize(stmt);
    return retVal;
}

bool sqlite3_command(sqlite3* DB, const char* query)
{
    // Prepare the statement.
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_statement(DB,&stmt,query)) return false;
    // Run the query.
    int src = sqlite3_run_query(DB,stmt);
    return src==SQLITE_DONE;
}

int IMSITable::open(const char* wPath)
{
    int rc = sqlite3_open(wPath,&mDB);
    if(rc){
        cout<< "cannot open IMSITable" << endl;
        sqlite3_close(mDB);
        mDB = NULL;
        return 1;
     }
     if(!sqlite3_command(mDB,createIMSITable)){
        cout<< "connot create IMSI table" <<endl;
        return 1;
     }
}

unsigned IMSITable::assign(Msg_Imsi* responseimsi,bool fAccess)
{
    char query[1000];
    sprintf(query,"INSERT INTO IMSI_TABLE(IMSI,HARDWARE,WlName,SerialNum,Time,Rsrq,UlCqi,UlRssi,Access)"
                  "VALUES('%s','%s','%s','%s','%s','%s','%s','%s','%d')",
                 responseimsi->imsi,responseimsi->cardid,responseimsi->railname,responseimsi->ID,responseimsi->time,responseimsi->rsrp,\
                 responseimsi->ulCqi,responseimsi->ulRssi,fAccess);
    if(!sqlite3_command(mDB,query)){
         LOG_ERROR("IMSI INSERT failed");
        return 0;
    }
}

IMSITable::~IMSITable()
{
        if(mDB)sqlite3_close(mDB);
}

