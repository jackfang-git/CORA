
#include "ctl_common.h"
#include "ctl_cmd_queue.h"

typedef struct node_t {
	struct node_t    * next;
	cmd_t          cmd;
} cmd_node_t;

typedef struct {
	cmd_node_t    * front;
	cmd_node_t    * rear;
} cmd_queue_t;

static cmd_queue_t cmd_queue;

bool ctl_cmd_queue_is_empty(void)
{
	return cmd_queue.front == NULL;
}

ret_code_t ctl_cmd_dequeue(cmd_t * p_cmd)
{
	cmd_node_t * p_node;

	VERIFY_PARAM_NOT_NULL(p_cmd);

	if(ctl_cmd_queue_is_empty())
		return NRF_ERROR_NOT_FOUND;

	p_node = cmd_queue.front;
	memcpy(p_cmd, &p_node->cmd, sizeof(cmd_t));

	if(cmd_queue.front->next == NULL) {  //there is only one node
		cmd_queue.front = NULL;
		cmd_queue.rear = NULL;
	} else {
		cmd_queue.front = cmd_queue.front->next;
	}

	free(p_node);

	return NRF_SUCCESS;
}

ret_code_t ctl_cmd_enqueue(const cmd_t * p_cmd)
{
	VERIFY_PARAM_NOT_NULL(p_cmd);

	cmd_node_t * p_node = malloc(sizeof(cmd_node_t));
	if(p_node == NULL) {
		return NRF_ERROR_NO_MEM;
	}

	memcpy(&p_node->cmd, p_cmd, sizeof(cmd_t));
	p_node->next = NULL;

	if(ctl_cmd_queue_is_empty()) {  //add front and rear node
		cmd_queue.front = p_node;
		cmd_queue.rear = p_node;
	} else {  //add new rear node
		cmd_queue.rear->next = p_node;
		cmd_queue.rear = p_node;
	}

	return NRF_SUCCESS;
}

void ctl_cmd_queue_init(void)
{
	cmd_queue.front = NULL;
	cmd_queue.rear = NULL;
}
