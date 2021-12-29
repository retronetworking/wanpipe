#ifndef ARRAY_QUEUE_H
#define ARRAY_QUEUE_H

//#define DEBUG_ARR_Q	DbgPrint
#define DEBUG_ARR_Q

#include <wanpipe_debug.h>

#define MINIMUM_LENGTH_OF_DATA_QUEUE	10
#define MAXIMUM_LENGTH_OF_DATA_QUEUE	16000	//2 Mbit, 1 byte frame + 2 CRC
#define MINIMUM_LENGTH_OF_DATA			10

typedef struct
{
	api_header_t api_header;

	PUCHAR data;

}DATA_QUEUE_ELEMENT;

typedef struct
{
	//maximum allowed number of nodes in the queue
	ULONG max_num_of_elements_in_q;
	//current number of nodes in the queue
	ULONG num_of_elements_in_q;

	USHORT max_data_length;

	//this flag will prevent from printing the same 'q full' message many times.
	UCHAR q_full_message_printed;

	//insert at tail
	ULONG f;
	//remove from head
	ULONG r;

	DATA_QUEUE_ELEMENT* elements_array[MAXIMUM_LENGTH_OF_DATA_QUEUE];

}DATA_QUEUE;

/////////////////////////////////////////////////////////////////////////
ULONG arr_q_size(DATA_QUEUE * q_ptr);

int is_empty(DATA_QUEUE * q_ptr);

DATA_QUEUE_ELEMENT * front(DATA_QUEUE * q_ptr);

DATA_QUEUE_ELEMENT * dequeue(DATA_QUEUE * q_ptr);

int enqueue(DATA_QUEUE * q_ptr, DATA_QUEUE_ELEMENT * new_element);
/////////////////////////////////////////////////////////////////////////

static __inline ULONG arr_q_size(DATA_QUEUE * q_ptr)
{
	ULONG q_size;

	DEBUG_ARR_Q("arr_q_size(): q_ptr: %p\n", q_ptr);
	DEBUG_ARR_Q("q_ptr->f: %d, q_ptr->r: %d\n", q_ptr->f, q_ptr->r);

	q_size = (q_ptr->max_num_of_elements_in_q - q_ptr->f + q_ptr->r) % q_ptr->max_num_of_elements_in_q;

	DEBUG_ARR_Q("q_size: %d\n", q_size);

	return q_size;
}


static __inline int is_empty(DATA_QUEUE * q_ptr)
{
	int rc;

	DEBUG_ARR_Q("is_empty(): q_ptr: %p\n", q_ptr);
	DEBUG_ARR_Q("q_ptr->f: %d, q_ptr->r: %d\n", q_ptr->f, q_ptr->r);

	rc = (q_ptr->f == q_ptr->r);

	DEBUG_ARR_Q("rc: %d\n", rc);

	return rc;
}
/*
static __inline DATA_QUEUE_ELEMENT * front(DATA_QUEUE * q_ptr)
{
	DEBUG_ARR_Q("front(): q_ptr: %p\n", q_ptr);

	if(is_empty(q_ptr))
	{
		return NULL;
	}else
	{	return &q_ptr->elements_array[q_ptr->f];
	}
}
*/

static __inline DATA_QUEUE_ELEMENT * dequeue(DATA_QUEUE * q_ptr)
{
	DEBUG_ARR_Q("dequeue(): q_ptr: %p\n", q_ptr);

	if(is_empty(q_ptr))
	{
		return NULL;
	}else
	{
		DATA_QUEUE_ELEMENT * temp = q_ptr->elements_array[q_ptr->f];

		DEBUG_ARR_Q("dequeue(): q_ptr->f: %d\n", q_ptr->f);

		//q_ptr->elements_array[q_ptr->f] = NULL;

		q_ptr->f = (q_ptr->f + 1) % q_ptr->max_num_of_elements_in_q;

		return temp;
	}
}

static __inline int enqueue(DATA_QUEUE * q_ptr, DATA_QUEUE_ELEMENT * new_element)
{
	DEBUG_ARR_Q("enqueue(): q_ptr: %p\n", q_ptr);

	if(arr_q_size(q_ptr) == q_ptr->max_num_of_elements_in_q - 1){

		DEBUG_ARR_Q("enqueue() : FULL!!\n");
		return 1;
	}

	DEBUG_ARR_Q("enqueue(): q_ptr->r: %d\n", q_ptr->r);
	DEBUG_ARR_Q("enqueue(): new_element->api_header.data_length : %d\n",
		new_element->api_header.data_length);

	//memcpy(q_ptr->elements_array[q_ptr->r], new_element, sizeof(DATA_QUEUE_ELEMENT));

	RtlCopyMemory(&q_ptr->elements_array[q_ptr->r]->api_header,
		&new_element->api_header, sizeof(api_header_t));

	//check data will fit into the buffer
	if(new_element->api_header.data_length > q_ptr->max_data_length){
		DEBUG_ARR_Q("enqueue() : data larger then queue buffer!!\n");
		return 2;
	}

	RtlCopyMemory(q_ptr->elements_array[q_ptr->r]->data,
		new_element->data, new_element->api_header.data_length);

	q_ptr->r = (q_ptr->r + 1) % q_ptr->max_num_of_elements_in_q;

	return 0;
}

static __inline int is_full(DATA_QUEUE * q_ptr)
{
	DEBUG_ARR_Q("is_full(): q_ptr: %p\n", q_ptr);

	if(arr_q_size(q_ptr) == q_ptr->max_num_of_elements_in_q - 1){
		DEBUG_ARR_Q("is_full() : FULL!!\n");
		return 1;
	}
	DEBUG_ARR_Q("is_full() : NOT full.\n");
	return 0;
}

#endif //ARRAY_QUEUE_H
