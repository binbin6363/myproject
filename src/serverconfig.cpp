/**
 * @filedesc: 
 * client_session.h, handle client action
 * @author: 
 *  bbwang
 * @date: 
 *  2014/9/26 11:02:59
 * @modify:
 *
**/

#include "serverconfig.h"
#include "mobile_server_app.h"
#include <sstream>
#include "codeconverter.h"


namespace toolutils{


ServerConfig::ServerConfig(void)
    : partfile_(NULL)
{
}

ServerConfig::~ServerConfig(void)
{
}

bool ServerConfig::load_file(string file)
{
    bool ret = true;
    ONCE_LOOP_ENTER
        appConfig = new TiXmlDocument;
        ret = appConfig->LoadFile(file);
        if (!ret)
        {
            ret = false;
            break;
        }
        RootElement = appConfig->RootElement();
        if (!RootElement)
        {
            ret = false;
            break;
        }
    ONCE_LOOP_LEAVE
    return ret;
}

bool ServerConfig::init_log()
{
    const TiXmlElement *cfgLog = RootElement->FirstChildElement("log");
    if (!cfgLog && !cfgLog->FirstChildElement("file") && !cfgLog->FirstChildElement("rank"))
        return false;

    string logFile = cfgLog->FirstChildElement("file")->GetText();
    string logRank = cfgLog->FirstChildElement("rank")->GetText();
    cout<<"open log file:"<<(const char*)logFile.c_str()<<endl;
    LOG_INIT((const char*)logFile.c_str(),500000000,utils::L_TRACE);
    LOG_OPEN();
    LOG_SET_USEC(true);

    if (logRank == "DEBUG")
    {
        LOG_SET_LEVEL(utils::L_DEBUG);
    }
    else if (logRank == "INFO")
    {
        LOG_SET_LEVEL(utils::L_INFO);
    }
    else if (logRank == "ERROR")
    {
        LOG_SET_LEVEL(utils::L_ERROR);
    }
    return true;

}





}

