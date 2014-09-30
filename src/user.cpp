/**
 * @filedesc: 
 * user.cpp
 * @author: 
 *  bbwang
 * @date: 
 *  2014/9/26 11:02:59
 * @modify:
 *
**/

#include "user.h"


namespace toolutils {

UserMgr &UserMgr::append_user(User user)
{
    if (NULL != user_list_.find(user.account_))
    {
        user_list_.push_back(user);
    }
}


void UserMgr::clear_user()
{
    user_list_.clear();
}


}

