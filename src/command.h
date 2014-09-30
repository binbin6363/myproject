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

#ifndef __COMMAND_H__
#define __COMMAND_H__

#include <string>
#include <map>
#include <list>

typedef struct word_cell_{
    int type;
    string name;
    string value;
}word_cell;

//<index, word_cell>
typedef std::map<int, word_cell> index_word;

typedef struct packet_
{
    CMHDR head_;
    index_word words_;
}packet;















#endif //__COMMAND_H__
