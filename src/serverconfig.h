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

#ifndef __SERVER_CONFIG_H__
#define __SERVER_CONFIG_H__
#include "comm.h"
#include "tinyxml/tinyxml.h"
#include <vector>
#include <string>
#include <map>

using namespace std;

namespace utils{

template <typename T>
class Singleton
{
public:
    static T &inst()
    {
        static T inst_;
        return inst_;
    }
};


class ServerConfig
{

// ensure singleton
private:
    ServerConfig(void);
    ~ServerConfig(void);
    ServerConfig &operator=(const ServerConfig &other);


public:
    static ServerConfig *Instance()
    {
        static ServerConfig server_config_;
        return &server_config_;
    }

    bool init_log();
    bool load_config();
    
    //const string &config_file() const {return config_file_;}
    bool  load_file(string file);//{config_file_ = file_;}

private:
    TiXmlDocument *appConfig;
};



}
#endif // __SERVER_CONFIG_H__
