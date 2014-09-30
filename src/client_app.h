/**
 * @filedesc: 
 * command.h
 * @author: 
 *  bbwang
 * @date: 
 *  2014/9/26 11:02:59
 * @modify:
 *
**/

#ifndef __CLIENT_APP_H__
#define __CLIENT_APP_H__

// app
class ClientApp
{
public:
    ClientApp();
    ~ClientApp();
    //int init(const char *address);
    int open();
    int run_service();
    int open_imdservice();
    int send_packet(const packet &pkg);

    int open_mobiservice();

    UserSession *get_session() {return mobi_session_;}
private:
    UserSession                  *mobi_session_;
    Net_Connector<UserSession>   mobi_connector_;
    Net_Manager *net_manager_;
    MobiSpliter mobi_splitter_;

};

ClientApp::ClientApp()
{
    mobi_session_ = new UserSession;
}

ClientApp::~ClientApp()
{
    if (NULL != mobi_session_)
        delete mobi_session_;
}
/*
int ClientApp::init(const char *address)
{
    return mobi_session_->init_addr(address);
}
*/
int ClientApp::open()
{
    int ret = 0;
    net_manager_ = new Net_Manager;
    // todo: start net thread, handle event
    if (0 != net_manager_->start())
    {
        LOG(ERROR)("net_manager_ start failed.");
        ret = -1;
        return ret;
    }

    mobi_connector_.open(net_manager_);
    LOG(INFO)("open mobi session");
    // open and collect db server session
    if (0 != open_mobiservice())
    {
        LOG(ERROR)("mobi_session open failed.");
        return -1;
    }
    return ret;

}

int ClientApp::open_mobiservice()
{
    LOG(INFO)("open_mobiservice");
    
   // LOG(DEBUG)("connect | mobi_session_:%u, addr:%s, timeout:%u.", db_session_, addr, dbp_serv.conn_time_out);
    if (mobi_connector_.connect(mobi_session_, mobi_session_->addr(), &mobi_splitter_, 120) != 0)
    {
        LOG(WARN)("connect mobi failed.");
        //连接失败，则重连
        mobi_session_->handle_close(0);
    }
    LOG(DEBUG)("connect mobi succeed.");
    return 0;
}

int ClientApp::run_service()
{
    if (!net_manager_)
    {
        LOG(WARN)("client app is not initialized.");
        return -1;
    }
    UserSession *mobi_session = dynamic_cast<UserSession *>(mobi_session_);

    while(1)
    {
        Net_Event*  ev = net_manager_->get_event();
        if (mobi_session->can_shutdown())
        {
            LOG(INFO)("teardown service thread.");
            break ;
        }
        if (ev)
        {
            ev->handler(*ev);
            delete ev;
        }
        else
        {
            usleep(10);
        }
    }
    return 0;
}


#endif //__CLIENT_APP_H__
