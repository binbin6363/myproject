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



namespace tool_util{

int Client::run_engine(uint32_t engine_num, uint32_t handle_num)
{
    for(int k=0; k<engine_num; k++)
	{

		pid_t pid = fork();
		if( pid > 0 )
		{
			continue;
		}
		else if( pid < 0 )
		{
			break;
		}
        collector_.collect();
		for(int i=0; i<handle_num; i++)
		{
			stCoRoutine_t *co = 0;
			co_create( &co,NULL,readwrite_routine, &endpoint);
			co_resume( co );
		}
		co_eventloop( co_get_epoll_ct(),0,0 );

		exit(0);
	}

}











}

