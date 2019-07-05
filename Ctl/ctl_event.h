#ifndef CTL_EVENT_H_
#define CTL_EVENT_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum 
{
    E_EVENT_NULL=0,
    E_EVENT_MAX,
} TE_EVENT;

typedef struct 
{
	TE_EVENT event;
    uint32_t param;
} TS_EVENT;

typedef void (*TF_EVENT_FUNC)(uint32_t param);

void    kpd_event_get(TS_EVENT *event_obj);
void    kpd_event_handle(TS_EVENT *event_obj);
void    kpd_event_handle_all(void);
void    kpd_event_post(TE_EVENT event, uint32_t param);
void    kpd_event_bind_handle(TE_EVENT event, TF_EVENT_FUNC func);
void    kpd_event_clear_all_handle(void);
bool    kpd_event_is_empty(void);



#endif
