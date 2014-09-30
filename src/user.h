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

#ifndef __USER_H__
#define __USER_H__
#include "serverconfig.h"


namespace toolutils {


class User
{
public:
    User(string account, string paswd)
        : account_(account)
        , paswd_(paswd)
    {
    }

    ~User()
    {
    }

private:
    string account_;
    string paswd_;
};

typedef std::list<User> UserList;

// singleton
class UserMgr : public Singleton<UserMgr>
{
public:
    UserMgr &add_user(User user);
    void clear_user();
private:
    UserList user_list_;
};


}
#endif //__USER_H__

