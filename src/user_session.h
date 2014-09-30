/**
 * @filedesc: 
 * user_session.h, handle user action
 * @author: 
 *  bbwang
 * @date: 
 *  2014/9/26 11:02:59
 * @modify:
 *
**/

#ifndef __USER_SESSION_H__
#define __USER_SESSION_H__

#include "comm.h"
#include "net_session.h"
#include "global.h"
#include "log.h"
#include "net_connector.h"
#include "data_type.h"
#include "tinyxml.h"
#include "xtea.h"
#include "md5.h"
#include <string>
#include <vector>
#include "dbconstant.h"
#include "codeconverter.h"
#include "command.h"

using namespace std;
using namespace CPPSocket;
using namespace utils;
using namespace DBCMD;

const static int PASSWD_LENGTH = 16;
const static int MAX_NET_LENGTH = 5*1024;



// session
class UserSession : public Net_Session
{
public:
    UserSession()
    : net_manager_(NULL)
    , uid_(0)
    , cid_(0)
    , ver_(0) 
    , seq_(0)
    , send_next_(false)
    , ttl_(10)
    , buffer_(NULL)
    , shutdown_(false)
    {}

    ~UserSession() {}
    virtual int open(void *arg, const INET_Addr &remote_addr);
    int send_to_mobi(packet *pkg);
    virtual int on_receive_message(char *ptr, int len);
    
    int init_addr(const char *address);
    INET_Addr addr(){return addr_;}
    virtual int handle_close(uint32_t handle);

    bool can_send(){return send_next_;}
    void hanup()   {send_next_ = false;}

    string &session_key() {return session_key_;}
    void set_session_key(const char *key) {session_key_ = key;}

    string &paswd() {return paswd_;}
    void set_paswd(string &paswd) {paswd_ = paswd;}

    virtual char *get_buffer(){return buffer_;}

    void shutdown(){ shutdown_ = true; }
    bool can_shutdown() {return shutdown_;}

    uint32_t ttl(){return ttl_;}
    void beat();


private:
    int extract_pkg(BinInputPacket<> &inpkg);
    int check_deciph_pkg(BinInputPacket<> &inpkg);
    inline int show_reply(BinInputPacket<> &inpkg)
    {
        int ret = 0;
        CMHDR head;
        inpkg.get_head(head);
        //fun(inpkg);
        LOG(DEBUG)("show msg %s", head.print());
        LOG_HEX(inpkg.getData(), inpkg.size(), utils::L_DEBUG);
        return ret;
    }

    void send_next(){send_next_ = true;}


private:
    INET_Addr   addr_;
    Net_Manager *net_manager_;
    uint32_t    uid_;
    uint32_t    cid_;
    uint8_t     ver_;
    uint32_t    seq_;
    bool        send_next_;
    uint32_t    ttl_;
    string      paswd_;
    string      session_key_;
    char *      buffer_;
    bool shutdown_;
};


void UserSession::beat()
{
    static char beat_buf[MAX_NET_LENGTH] = {0};
    CMHDR cmhdr;
    cmhdr.unLen = sizeof(CMHDR);
    cmhdr.unCmd = 1001;
    cmhdr.unCid = cid_;
    cmhdr.unUid = uid_;
    cmhdr.unVer = ver_;
    cmhdr.unSeq = seq_;
    BinOutputPacket<> outpkg(beat_buf, MAX_NET_LENGTH);
    outpkg.offset_head(sizeof(CMHDR));
    outpkg.set_head(cmhdr);
    int ret = this->send_msg(outpkg.getData(), cmhdr.unLen);
}

int UserSession::on_receive_message(char *ptr, int len)
{
    int ret = 0;
    BinInputPacket<> inpkg(ptr, len);
    CMHDR cmhdr;
    inpkg.offset_head(sizeof(CMHDR));
    inpkg.get_head(cmhdr);
    fprintf(stdout, "on_receive_message| data len:%d, [%s]\n", len, cmhdr.print());
    LOG(INFO)("receive message, [head info] len=%u, cmd=%u, seq=%u, ver=%u, cid=%u, uid=%u."
    , cmhdr.unLen, cmhdr.unCmd, cmhdr.unSeq, cmhdr.unVer, cmhdr.unCid, cmhdr.unUid);
    LOG_HEX(inpkg.getData(), cmhdr.unLen, utils::L_DEBUG);
    if (!inpkg.good())
    {
        ret = -1;
        LOG(ERROR)("on_receive_message, get head failed.");
    }
    // deciph pkg
    // 第一个包用密码的md5串解密，之后都是用服务端传来的key解密
    check_deciph_pkg(inpkg);
    switch (cmhdr.unCmd)
    {
        /*
            pwd.encrypt((uint16_t)ret + buffer)
            buffer = (uint32_t)cid + (uint32_t)uid ＋ (char16)sessionkey + (uint32_t)corpuc 
            +(uint32_t) regular_authority + (uint32_t) im_authority
             + (uint16_t)num + [(uint32_t)uploadip + (uint16_t)uploadport] + (uint32_t) time
        */
        case NEW_LOGIN_CMD:
        {
            cid_ = cmhdr.unCid;
            uid_ = cmhdr.unUid;
            ver_ = cmhdr.unVer;
            seq_ = cmhdr.unSeq;
            LOG(DEBUG)("cid:%u, uid:%u, ver:%u, seq:%u", cid_, uid_, ver_, seq_);
            if (0 != extract_pkg(inpkg))
            {
                shutdown();
                break;
            }
            //uint32_t cmd = cmhdr.unCmd;
            //pfun fun = SHOW(cmd);
            //ret = show_reply(fun, inpkg);
            ret = show_reply( inpkg);
            sleep(2);
            send_next();
            break;
        }
        case 1001:    
        {
            fprintf(stdout, "client alive!\n");
            break;
        }
        case 1004:
        {
            LOG(DEBUG)("kicked by another user");
            fprintf(stdout, "kicked by another user!\n");
            shutdown();
            break;
        }
        /*
        randkey.encrypt(buffer);
        case SYNC_DEPT_INFO:     
        case FETCH_DEPT_COUNT:        
        case SYNC_QGROUP_LIST:       
        case SYNC_QGROUP_USER_LIST:   
        case SYNC_NGROUP_LIST:        
        case SYNC_NGROUP_USER_LIST:   
        case SYNC_CONTACTS_LIST:      
        case SYNC_CONTACTS_USER_LIST: 
        case FETCH_USER_INFO:
        {
        }
        */
        default:
        {
            //uint32_t cmd = cmhdr.unCmd;
            //ret = show_reply(SHOW(cmd), inpkg);
            ret = show_reply( inpkg);
            send_next();
        break;
//            LOG(ERROR)("ERROR!!! cmd=%u not supported.", cmhdr.unCmd);
//            break;
        }
    }
    return ret;
}

int UserSession::extract_pkg(BinInputPacket<> &inpkg)
{
    int ret = 0;
    uint32_t corp_uc = 0;
    uint32_t regular_authority = 0;
    uint32_t im_authority = 0;
    uint16_t num = 0;
    uint32_t time = 0;
    LOG_HEX(inpkg.getData(), inpkg.size(), utils::L_DEBUG);
    ONCE_LOOP_ENTER
        uint16_t ret_value = 0;
        inpkg >> ret_value;
        if (0 != ret_value)
        {
            fprintf(stderr, "login failed! ret=%u\n", ret_value);
            LOG(ERROR)("login failed, ret:%u", ret_value);
            ret = -1;
        }
        inpkg >> cid_ >> uid_;
        char key[PASSWD_LENGTH] = {0};
        inpkg.read(key, PASSWD_LENGTH);
        session_key_ = string(key);
        LOG(DEBUG)("session key str:%s", session_key().c_str());
        LOG(DEBUG)("session key hex:");
        LOG_HEX(session_key().c_str(), 16, utils::L_DEBUG);
        inpkg >> num;
        uint32_t uploadip = 0;
        uint16_t uploadport = 0;
        for (uint16_t i = 0; i < num; ++i)
        {
            inpkg >> uploadip >> uploadport;
            LOG(DEBUG)("uploadip:%u, uploadport:%u", uploadip, uploadport);
        }
        inpkg >> time >> corp_uc >> regular_authority >> im_authority;
        LOG(DEBUG)("ret:%u, cid:%u, uid:%u, corp_uc:%u, regular_authority:%u, im_authority:%u, num:%u"
        , ret_value, cid_, uid_, corp_uc, regular_authority, im_authority, num);
        LOG(DEBUG)("login succeed server time:%u", time);
        if (!inpkg.good())
        {
            ret = -1;
            LOG(ERROR)("extract session key failed.");
            break;
        }
    ONCE_LOOP_LEAVE
    return ret;
}

// check whether the pkg take body, decipher it when have body.
int UserSession::check_deciph_pkg(BinInputPacket<> &inpkg)
{
    int ret = 0;
    LOG(DEBUG)("[uid:%u] UserSession::check_deciph_pkg", uid_);
    CMHDR cmhdr;
    inpkg.get_head(cmhdr);
    LOG_HEX(inpkg.getData(), inpkg.size(), utils::L_DEBUG);
    if( cmhdr.unLen > sizeof(CMHDR) )
    {
        LOG(DEBUG)("[uid:%u] [start decip session key]:", uid_);
        LOG(DEBUG)("paswd:%s, decip session key:%s", paswd_.c_str(), session_key_.c_str());
        int out_len = 0;
        XTEA::decipher(session_key().c_str()
            , inpkg.getCur() 
            , cmhdr.unLen - sizeof(CMHDR) 
            , inpkg.getCur() 
            , cmhdr.unLen - sizeof(CMHDR)
            , out_len);

        inpkg.set_pkglen(out_len + sizeof(CMHDR) );
        LOG(DEBUG)("decipher outlen=%d, [uid:%u] sesseionkey:", out_len, uid_);
        LOG(DEBUG)("after decode pkg:");
        LOG_HEX(inpkg.getData(), inpkg.size(), utils::L_DEBUG);
    }
    return ret;
}


int UserSession::init_addr(const char *address)
{
    LOG(DEBUG)("dbp address info:%s", address);
    if(FromStringToAddr(address, addr_) == -1){
        LOG(ERROR)("parse addr error,service address=%s", address);
        return -1;
    }
    return 0;
}

int UserSession::open(void *arg, const INET_Addr &remote_addr)
{
    LOG(INFO)("ClientSession::open, ip:%s, handle:%u.", FromAddrTostring(remote_addr).c_str(), handle());
    if (NULL == buffer_)
        buffer_ = new char[MAX_NET_LENGTH];
    Net_Session::open(arg, remote_addr);
    send_next();

    return 0;
}

int UserSession::handle_close(uint32_t handle) 
{
    LOG(INFO)("ClientSession::handle_close,csessionid:%s, handle:%u, ip:%s"
    , session_id().c_str(), handle, FromAddrTostring(remote_addr()).c_str());
    delete this;
    return 0;
}

// 第一次用randkey加密，会传出密码的md5串，第二次用服务端传来的key加密，不传密码
int UserSession::send_to_mobi(packet *pkg)
{
    // generate packet
    BinOutputPacket<> outpkg(get_buffer(), MAX_NET_LENGTH);
    uint32_t have_paswd = 0;
    outpkg.offset_head(sizeof(CMHDR));
    CMHDR cmhdr;
    word_cell *word_c = NULL;
    cmhdr = pkg->head_;
    cmhdr.unCid = cid_;
    cmhdr.unUid = uid_;
    cmhdr.unSeq = ++seq_;
    cmhdr.unVer = ver_;
    if (NEW_LOGIN_CMD == cmhdr.unCmd)
    {
        have_paswd = PASSWD_LENGTH;
        LOG(DEBUG)("login, pkg session key:%s", session_key().c_str());
        char random_key[16] = {0};//const_cast<char*>(session_key().c_str())
        strcpy(random_key, session_key().c_str());
        outpkg.write(random_key, PASSWD_LENGTH);
    }
    for (uint32_t i = 0; i < pkg->words_.size(); ++i)
    {
        word_c = &(pkg->words_.at(i));
        LOG(DEBUG)("type:%u, value:%s", word_c->type, word_c->value.c_str());
        switch(word_c->type)
        {
            case 1:
                if (0 == strcmp("0", word_c->value.c_str()))
                    outpkg << (uint8_t)(0);
                else
                    outpkg << (uint8_t)atoi(word_c->value.c_str());
                break;
            case 2:
                if (0 == strcmp("0", word_c->value.c_str()))
                    outpkg << (uint16_t)(0);
                else
                    outpkg << (uint16_t)atoi(word_c->value.c_str());
               break;
            case 3:
                if (0 == strcmp("0", word_c->value.c_str()))
                    outpkg << (uint32_t)(0);
                else
                    outpkg << (uint32_t)atoi(word_c->value.c_str());
                break;
            case 4:
                if (0 == strcmp("0", word_c->value.c_str()))
                    outpkg << (uint64_t)(0);
                else
                {
                    uint64_t u64 = 0;
                    sscanf( word_c->value.c_str() , "%llu" , &u64 );
                    outpkg << u64;
                    //fprintf(stdout, "u64 str:%s, u64:%llu\n", word_c->value.c_str(), u64);
                }
                break;
            case 5:
            {
                CCodeConverter converter;
                string unicode_str = "";
                converter.Convert("UTF-8", "UNICODELITTLE", word_c->value.c_str(), unicode_str);
                LOG(DEBUG)("ASCII STR:%s, UNICODE STR:%s", word_c->value.c_str(), unicode_str.c_str());

                // passwd md5 encrypt
                if ("passwd" == word_c->name)
                {
                    LOG(DEBUG)("encrypt passwd, %s", word_c->value.c_str());
                    char paswd[PASSWD_LENGTH] = {0};
                    md5((const unsigned char*)word_c->value.c_str(), paswd, word_c->value.length());
                    outpkg.write(paswd, PASSWD_LENGTH);
                    paswd_ = paswd;
                    LOG(DEBUG)("MD5 PASSWD STRING:%s", paswd_.c_str());
                    LOG(DEBUG)("MD5 PASSWD HEX");
                    LOG_HEX(paswd_.c_str(), PASSWD_LENGTH, utils::L_DEBUG);
                }
                else
                {
                    LOG(DEBUG)("NOT PASSWD, %s", unicode_str.c_str());
                    outpkg << unicode_str;
                }
            }
            break;
        default:
            LOG(ERROR)("BODY DATA ERROR!");
        break;
        }
    }
    int nOutLen = 0;
    if (17 < outpkg.length())
    {
        LOG(INFO)("send pkg to mobiled unencrypt. [%s]", cmhdr.print());
        LOG_HEX(outpkg.getData(), outpkg.length(), utils::L_DEBUG);
        XTEA::encipher(session_key().c_str()
            , outpkg.getData() + sizeof(CMHDR) + have_paswd
            , outpkg.length() - sizeof(CMHDR) - have_paswd
            , outpkg.getData() + sizeof(CMHDR) + have_paswd
            , 4500
            , nOutLen);
    }
    cmhdr.unLen = (uint16_t)(nOutLen + sizeof(CMHDR) + have_paswd);
    outpkg.set_head(cmhdr);
    //LOG(INFO)("send pkg to mobiled. [%s]", cmhdr.print());
    //LOG_HEX(outpkg.getData(), cmhdr.unLen, utils::L_DEBUG);
    if (!outpkg.good())
    {
        LOG(ERROR)("send_to_mobi | output pkg is error!!!");
    }
    fprintf(stdout, "send_to_mobi| [%s]\n\n", cmhdr.print());
    if (1003 == cmhdr.unCmd)
    {
        sleep(2);
    }
    int ret = this->send_msg(outpkg.getData(), cmhdr.unLen);
    if (NEW_LOGIN_CMD == cmhdr.unCmd)
    {
        session_key_ = paswd_;
    }
    
    return ret;
}

class MobiSpliter : public Packet_Splitter
{

public:
virtual int split( const char *buf, int len, int &packet_begin, int &packet_len )
{
    if ((size_t) len < sizeof(CMHDR) )
    {
        //LOG(ERROR)("pkg len:%u too short.", len);
        return 0;
    }

    CMHDR *hdr = (CMHDR *) buf;
    uint32_t unLen = ntohs(hdr->unLen);
    uint32_t unCmd = ntohs(hdr->unCmd);
    uint32_t unSeq = ntohl(hdr->unSeq);
    uint32_t unVer = hdr->unVer;
    uint32_t unCid = ntohl(hdr->unCid);
    uint32_t unUid = ntohl(hdr->unUid);
    LOG(INFO) ("MobiSpliter. hdr.unLen:%u, pkglen:%d, cmd:%u, seq:%u, ver:%u, cid:%u, uid:%u!",
        unLen, len, unCmd, unSeq, unVer, unCid, unUid);
    LOG_HEX(buf, len, utils::L_DEBUG);

    if( unLen > MSGCENTER_COND_NET_PKG_MAX_LEN || unLen < sizeof(CMHDR))
    {
        LOG(ERROR) ("MobiSpliter split error, pkg too long or short. hdr.unLen:%u, pkglen:%d, cmd:%u, seq:%u, ver:%u, cid:%u, uid:%u!",
            unLen, len, unCmd, unSeq, unVer, unCid, unUid);
        return -1;
    }

    if((int)unLen > len)
    {
        return 0;
    }
    if (len >= (int) unLen)
    {
        packet_begin = 0;
        packet_len = (int) unLen;

        return 1;
    } 

    return 1; 
}
};



#endif //__USER_SESSION_H__
