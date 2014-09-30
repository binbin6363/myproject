/**
 * @filedesc: 
 * client_engine.h, handle client action
 * @author: 
 *  bbwang
 * @date: 
 *  2014/9/26 11:02:59
 * @modify:
 *
**/
#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <tr1/memory>
using namespace std;

namespace tool_util{

struct Task
{
    Task()
        : id(0)
        {}
    
    ~Task()
        {
        id = 0;
        }
    int id;
};
typedef shared_ptr<Task> task_prt;


// 任务分发器
struct Dispatcher
{
public:
    virtual int dispatch(void) = 0;
    const task_prt &next(const task_prt &task) = 0;
    bool empty() = 0;
private:
    virtual ~Dispatcher();
};


// 信息收集器
struct Collector
{
public:
    virtual int collect(void) = 0;
private:
    virtual ~Collector();
};


// 任务配置器
struct Configer
{
public:
    virtual int config(void) = 0;
private:
    virtual ~Configer();
};



// define
typedef shared_ptr<Dispatcher> dispatcher_ptr;
typedef shared_ptr<Collector>  collector_ptr;
typedef shared_ptr<Configer>   configer_ptr;

// main handle engine
class Client
{

public:
    /**
    *
    * uint32_t engine_num 处理引擎数目
    * uint32_t handle_num 每个引擎处理的事件个数
    * return int
    **/
    int run_engine(uint32_t engine_num, uint32_t handle_num);


private:
    // 任务分发者
    dispatcher_ptr dispatcher_;
    // 性能收集器
    collector_ptr  collector_;
    // 任务配置器
    configer_ptr   configer_;
};


}// namespace tool_util


#endif //__CLIENT_H__

