/**
 * @filedesc: 
 * dispatcher.h
 * @author: 
 *  bbwang
 * @date: 
 *  2014/9/26 11:02:59
 * @modify:
 *
**/

#ifndef __DISPATCHER_H__
#define __DISPATCHER_H__

// collect packets and send packets
class Dispatcher : public utils::Task_Base
{
public:
    Dispatcher();
    ~Dispatcher();

    int collect_packets(TiXmlElement *root_element);
    void set_session(Net_Session *session){session_ = session;}

    //! 启动线程
    int start();
    //! 停止线程
    int stop();

    //! 线程函数
    virtual int svc();

private:

    int                        index_;
    int                        total_;
    Net_Session                *session_;
    //vector<BinOutputPacket<> *> pkgs_;
    //! 运行状态
    bool                       m_is_run;
    bool                       m_notify_stop;
    vector<packet>             packets_;
};

Dispatcher::Dispatcher()
    : index_(0)
    , total_(0)
    , session_(NULL)
    , m_is_run(false)
    , m_notify_stop(false)
{
    //pkgs_.reserve(10);
}

Dispatcher::~Dispatcher()
{
    
}


int Dispatcher::collect_packets(TiXmlElement *root_element)
{
    TiXmlElement *host_element = root_element->FirstChildElement("host");
    if (NULL == host_element)
    {
        LOG(ERROR)("get host element failed.");
        return -2;
    }
    char address[30] = {0};
    strcpy(address, host_element->GetText());
    LOG(DEBUG)("get address:%s.", address);
    UserSession *session = dynamic_cast<UserSession *>(session_);
    session->init_addr(address);
    TiXmlElement *cmd_element = NULL;
    TiXmlElement *head_element = NULL;
    TiXmlElement *body_element = NULL;
    TiXmlElement *item_element = NULL;
    cmd_element = root_element->FirstChildElement("cmd");
    // loop collect cmd
    int cnt = 0;
    int ret = 0;
    while (true)
    {
        packet pkg;
        if (NULL == cmd_element)
        {
            LOG(DEBUG)("done.");
            ret = 0;
            break;
        }
        head_element = cmd_element->FirstChildElement("head");
        if (NULL == head_element)
        {
            LOG(ERROR)("get head element failed.");
            ret = -2;
            break;
        }
        item_element = head_element->FirstChildElement("item");
        if (NULL == item_element)
        {
            LOG(ERROR)("get item element failed.");
            ret = -2;
            break;
        }

        CMHDR cmhdr;
        const char *word = item_element->GetText();
        cmhdr.unCmd = (uint16_t)atoi(word);
        LOG(INFO)("head info:unCmd:%u", cmhdr.unCmd);
        pkg.head_ = cmhdr;
        LOG(DEBUG)("get the %dst cmd", ++cnt);
        
        TiXmlElement *randkey_element = cmd_element->FirstChildElement("randkey");
        if (NULL != randkey_element)
        {
            string session_key = randkey_element->GetText();
            session->set_session_key(session_key.c_str());
        }
        body_element = cmd_element->FirstChildElement("body");
        if (NULL == body_element)
        {
            packets_.push_back(pkg);
            LOG(DEBUG)("packet size:%d", (int)packets_.size());
            cmd_element = cmd_element->NextSiblingElement("cmd");
            ++total_;
            continue;
        }
        item_element = body_element->FirstChildElement("item");
        if (NULL == item_element)
        {
            ret = 0;
            continue;
        }
        // <!--  type, 1:uint8_t, 2:uint16_t, 3:uint32_t, 4:uint64_t, 5:string -->
        int i = 0;
        while (item_element)
        {
            if (NULL == item_element )
            {
                LOG(DEBUG)("done.");
                break;
            }
            const char *item_type = item_element->Attribute("type");
            pkg.words_[i].value = item_element->GetText();
            pkg.words_[i].type = atoi(item_type);
            pkg.words_[i].name = item_element->Attribute("name");
            item_element = item_element->NextSiblingElement("item");
            LOG(DEBUG)("value:%s, name:%s, type:%d", pkg.words_[i].value.c_str(), pkg.words_[i].name.c_str(), pkg.words_[i].type);
            ++i;
        }
        packets_.push_back(pkg);
        LOG(DEBUG)("packet size:%d", (int)packets_.size());
        cmd_element = cmd_element->NextSiblingElement("cmd");
        ++total_;
    }
    return ret;
}


// 对用户循环发送请求
int Dispatcher::svc()
{
    LOG(INFO)("start dispatch task...");
    packet *pkg = NULL;
    UserSession *mobi_session = dynamic_cast<UserSession *>(session_);
    time_t last = time(NULL);
    time_t now = last;
    while (true)
    {
        now = time(NULL);
        if (mobi_session->can_shutdown())
        {
            LOG(INFO)("tear down");
            fprintf(stdout, "tear down\n");
            break;
        }
        else if (now - last > mobi_session->ttl())
        {
            LOG(INFO)("heart beat...");
            fprintf(stdout, "heart beat...\n");
            mobi_session->beat();
            last = now;
            continue;
        }

        if(mobi_session->can_send())
        {
            if (index_ < total_)
            {
                LOG(DEBUG)("GET PKG SEND...");
                pkg = &packets_[index_++];
                LOG(INFO)("send packet:%u", index_);
                LOG(DEBUG)("svc()| HEAD:[%s]", pkg->head_.print());
                mobi_session->send_to_mobi(pkg);
                sleep(1);
                if (OFFLINE_CMD == pkg->head_.unCmd)
                {
                    LOG(DEBUG)("active offline ...");
                    fprintf(stdout, "active offline ...\n");
                    mobi_session->shutdown();
                    break;
                }
            }
            else
            {
                mobi_session->hanup();
                LOG(INFO)("end dispatch, task done!");
                // wait receive pkg
                //sleep(6);
                //mobi_session->shutdown();
            }
            /*
            if (1007 != pkg->head_.unCmd)
            {
                mobi_session->hanup();
            }
            */
        }
        else
        {
            sleep(1);
        }
    }
    fprintf(stdout, "teardown send thread\n");
    LOG(INFO)("teardown send thread.");
    return 0;
}

int Dispatcher::start()
{
    if (false != m_is_run) {
        return -1;
    }

    m_notify_stop = false;
    int rc = activate();
    if (rc != 0) {
        return -1;
    }

    m_is_run = true;
    return 0;
}

int Dispatcher::stop()
{
    m_notify_stop = true;
    wait();

    m_is_run = false;
    return 0;
}

#endif //__DISPATCHER_H__
