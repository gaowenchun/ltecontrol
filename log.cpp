#include <log4cplus/logger.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/layout.h>
#include <log4cplus/configurator.h>
#include <log4cplus/loggingmacros.h>
#include<log4cplus/fileappender.h>   
#include "includes/log.h"           

Logger logger = Logger::getInstance(LOG4CPLUS_TEXT("main"));

void InitLogger()    
{    
    /*step1:Instantiateanappenderobject*/

    //SharedAppenderPtr _append(new FileAppender("lte_weilan.log"));
    SharedObjectPtr<Appender> _append (new ConsoleAppender());   
    _append->setName("filelogtest");
    
    std::string pattern = "%d{%m/%d/%y  %H:%M:%S} [%p] [%l] %m %n";

    std::auto_ptr<Layout> _layout(new PatternLayout(pattern));
    
    /* step 3: Attach the layout object to the appender */
    _append->setLayout( _layout );

    /*step4:set log level*/

    logger.setLogLevel(INFO_LOG_LEVEL);
    
    /*step5:Attachtheappenderobjecttothelogger*/

    logger.addAppender(_append);

      
      
} 
