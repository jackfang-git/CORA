#include <assert.h>
#include "ctl_event.h"
#include "app_util_platform.h"
#include "ctl_common.h"

#define EVENT_MAX_NUM 100 //50
#if EVENT_MAX_NUM<E_EVENT_MAX*2
#error event buffer is too samll
#endif

static TS_EVENT g_event_queue[EVENT_MAX_NUM] = {E_EVENT_NULL};
static uint8_t  g_event_queue_begin_index = 0;
static uint8_t  g_event_queue_end_index = 0;
static TF_EVENT_FUNC  g_event_handle_fun_arr[E_EVENT_MAX];

void kpd_event_get(TS_EVENT *event_obj)
{
    uint8_t end_index=0;
    end_index = g_event_queue_end_index;
    *event_obj = g_event_queue[g_event_queue_begin_index];
    if(g_event_queue_begin_index==end_index)
    {
        event_obj->event = E_EVENT_NULL;
    }
    else
    {
        g_event_queue[g_event_queue_begin_index].event = E_EVENT_NULL;
        if(g_event_queue_begin_index < EVENT_MAX_NUM-1)
        {
            g_event_queue_begin_index++;
        }
        else
        {
            g_event_queue_begin_index = 0;
        }
    } 
}


void kpd_event_handle(TS_EVENT *event_obj)
{
    if(event_obj->event!=E_EVENT_NULL && g_event_handle_fun_arr[event_obj->event])
    {
        g_event_handle_fun_arr[event_obj->event](event_obj->param);
    }
}
void kpd_event_handle_all(void)
{
    TS_EVENT event;
    kpd_event_get(&event);
	if(g_event_queue_end_index==50||g_event_queue_end_index==1)
	{
		//flex_debug("\r\n.........kpd_event_handle_all %d",g_event_queue_end_index);
	}
    while(event.event != E_EVENT_NULL)
    {
		//flex_debug("\r\nhe %d",event.event);
        #ifndef FLEX_MF_TEST
            //reset_timeout_start_time();
            #endif
        kpd_event_handle(&event);
        kpd_event_get(&event);
    }    
}


void kpd_event_post(TE_EVENT event, uint32_t param)
{
    uint8_t next_index=0;
    CRITICAL_REGION_ENTER();
    next_index = g_event_queue_end_index;
    next_index++;
    if(next_index >= EVENT_MAX_NUM)
    {
        next_index = 0;
    }
    if(next_index==g_event_queue_begin_index)
    {
		flex_debug("\r\n queue full");
    }
    else
    {
        g_event_queue[g_event_queue_end_index].event = event;
        g_event_queue[g_event_queue_end_index].param = param;
        g_event_queue_end_index = next_index;
    }
    CRITICAL_REGION_EXIT();
}


void kpd_event_bind_handle(TE_EVENT event, TF_EVENT_FUNC func)
{
    g_event_handle_fun_arr[event] = func;
}

void kpd_event_clear_all_handle(void)
{
    uint8_t event;
    for(event=0;event<E_EVENT_MAX;event++)
    {
        g_event_handle_fun_arr[(TE_EVENT)event] = NULL;
    }
}

bool kpd_event_is_empty(void)
{
    if(g_event_queue_begin_index==g_event_queue_end_index)
    {
        return true;
    }
    else
    {
        return false;
    }
} 

