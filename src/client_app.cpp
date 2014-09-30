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

#include <stdio.h>
#include <stdlib.h>
#include "bbutils.hpp"
#include "tinyxml/tinyxml.h"
#include "data_type.h"
#include "client_engine.h"

using namespace std;



void welcome();
void goodbye();
void help();
void hello();

bool SetLogger(const TiXmlElement& setting)
{
    const TiXmlElement *cfgLog = setting.FirstChildElement("log");
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


// xml file, load cmd
int load_cfg(const char *path_name, Dispatcher &dispatcher)
{
    int ret = 0;
    fprintf(stdout, "load_cfg, cfg path=%s\n", path_name);
    TiXmlDocument *documet= new TiXmlDocument(path_name);
    if(!documet || !documet->LoadFile())
    {
        LOG(ERROR)("load xml file failed.");
        fprintf(stderr, "load_cfg load xml file failed\n");
        return -1;
    }

    TiXmlElement *RootElement = documet->RootElement();
    if (NULL != RootElement && !SetLogger(*RootElement))
    {
        LOG(ERROR)("get roor element failed.\n");
        fprintf(stderr, "load_cfg get roor element failed");
        return -2;
    }

    dispatcher.collect_packets(RootElement);
    ////////////////////////////////////////////////
    return ret;
}

// �����û���
int load_user_info(const char *path_name)
{
    int ret = 0;
    fprintf(stdout, "load_cfg, cfg path=%s\n", path_name);
    TiXmlDocument *documet= new TiXmlDocument(path_name);
    if(!documet || !documet->LoadFile())
    {
        LOG(ERROR)("load xml file failed.");
        fprintf(stderr, "load_cfg load xml file failed\n");
        return -1;
    }

    TiXmlElement *RootElement = documet->RootElement();
    if (NULL != RootElement && !SetLogger(*RootElement))
    {
        LOG(ERROR)("get roor element failed.\n");
        fprintf(stderr, "load_cfg get roor element failed");
        return -2;
    }

    dispatcher.collect_packets(RootElement);
    ////////////////////////////////////////////////
    return ret;

}


// use libco
int main(int argc, char *argv[])
{
    int ret = 0;
    if (argc != 2)
    {
        ret = -1;
        return ret;
    }
    if (0 == strcmp(argv[1], "-hello"))
    {
        hello();
        fprintf(stdout, "\n");
        help();
        return ret;
    }
    welcome();
    // 1.load config
    char *path = argv[1];
    load_cfg(path);
    load_user_info(user_cfg_path);
    // libco handle event
    uint32_t engine_num = 0;
    uint32_t handle_num = 0;
    shared_ptr client_ptr(new Client);
    client_ptr.run_engine(engine_num, handle_num);

    //app.send_packet(outpkg);
    goodbye();

    return ret;
}

void welcome()
{
    fprintf(stdout, "*****************************************************\n");
    fprintf(stdout, "*                       welcome                     *\n");
    fprintf(stdout, "*                         ^-^                       *\n");
    fprintf(stdout, "*****************************************************\n");
}

void goodbye()
{
    fprintf(stdout, "*****************************************************\n");
    fprintf(stdout, "*                        ~^~                        *\n");
    fprintf(stdout, "*                      goodbye                      *\n");
    fprintf(stdout, "*****************************************************\n");
}

void help()
{
    fprintf(stdout, "*****************************************************\n");
    fprintf(stdout, "*  usage:                                           *\n");
    fprintf(stdout, "*  sendcmd cmd.xml                                  *\n");
    fprintf(stdout, "*  sendcmd -hello                                   *\n");
    fprintf(stdout, "*  copy right. @2014 imo ltd.                       *\n");
    fprintf(stdout, "*****************************************************\n");
}

void hello()
{
    fprintf(stdout, "*****************************************************\n");
    fprintf(stdout, "*                  hello girl                       *\n");
    fprintf(stdout, "*****************************************************\n");
}



/**
    ClientApp app;
    Dispatcher dispatcher;
    Net_Session *session = app.get_session();
    dispatcher.set_session(session);
    ret = load_cfg(path, dispatcher);
    LOG(INFO)("load config file done. ret=%d", ret);
    if (0 != ret)
    {
        fprintf(stderr, "error!!!\n");
        return 0;
    }
    const char *user_list = "";
    ret = load_user_info(user_list);

    // 2.open server session
    app.open();
    LOG(INFO)("init and open mobi server done.");

    // 3.start send packet thread
    dispatcher.start();
    LOG(INFO)("start send packet thread.");

    // 4.loop event
    LOG(INFO)("enter loop, begin...");
    app.run_service();
    LOG(INFO)("done.");


**/






