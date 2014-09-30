/**
 * @filedesc: 
 * engine.h, observer control center
 * @author: 
 *  bbwang
 * @date: 
 *  2014/9/26 11:02:59
 * @modify:
 *
**/

#ifndef __ENGINE_H__
#define __ENGINE_H__














class EngineCC
{
public:
    void add_observer();
    int  start_engine();
    // 启动每个观察者的间隔
    int  set_start_interval();
};

#endif // __ENGINE_H__
