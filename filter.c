#include <stdlib.h>
#include <pthread.h>

#include "descriptors.h"

int framefilter_init(framefilter_que_t* que)
{
	que->count = 0;
	que->top = (framefilter_descriptor_t*)NULL;
	
	pthread_mutex_init(&que->mutex, NULL);
	
	return 0;
}

int framefilter_add(framefilter_que_t* que, struct can_frame* frame)
{
	int i, ret = -1;
	framefilter_descriptor_t* desc_mem = (framefilter_descriptor_t*)malloc(sizeof(framefilter_descriptor_t));
	framefilter_descriptor_t* fptr;
	
	if(desc_mem == NULL)
		return ret;
	
	pthread_mutex_lock(&que->mutex);
	// copy to allocated memory
	desc_mem->frame = *frame;
	
	// link que
	if(que->count == 0)
	{
//		printf("framefilter_add: to top\n");
		// link to empty que
		que->count = 1;
		que->top = desc_mem;
		desc_mem->next = NULL;
	}
	else
	{
		// link to que bottom
		fptr = que->top;
		for(i=0; i<que->count; i++)
		{
			if(fptr->next == NULL)
			{
				fptr->next = desc_mem;
				desc_mem->next = NULL;
				que->count++;
				ret = 0;
				break;
			}
			fptr = fptr->next;
		}
	}
	pthread_mutex_unlock(&que->mutex);
	
	return ret;
}

int framefilter_remove(framefilter_que_t* que, framefilter_descriptor_t* desc)
{
	framefilter_descriptor_t* fptr;
	framefilter_descriptor_t* fptr_next;
	int i, ret = -1;
	
	fptr = que->top;
	if(desc == que->top)
	{
		que->top = fptr->next;
		que->count--;
		return 0;
	}
	
	for(i=0; (fptr != NULL) && (que->count < i); fptr=fptr->next, i++)
	{
		if(fptr->next == desc)
		{
			fptr_next = (framefilter_descriptor_t*)fptr->next;
			
			fptr->next = fptr_next->next;
			que->count--;
			
			ret = 0;
			break;
		}
	}
	return ret;
}

bool is_framefilter_block(framefilter_que_t* que, struct can_frame* frame)
{
	framefilter_descriptor_t* framefilter_desc;
	int i, j, ret = false;
	
	pthread_mutex_lock(&que->mutex);
	
	framefilter_desc = que->top;
	printf("is_framefilter_block: que in %d filters\n", que->count);
	
	for(i=0; (framefilter_desc != NULL) && (que->count > i); i++, framefilter_desc = framefilter_desc->next)
	{
//		printf("is_framefilter_block: %d, %X", i, framefilter_desc->frame.can_id);
		if(framefilter_desc->frame.can_id != frame->can_id)
		{
//			printf("\n");
			continue;
		}
//		printf(", %X", i, framefilter_desc->frame.can_dlc);
		if(framefilter_desc->frame.can_dlc != frame->can_dlc)
		{
//			printf("\n");
			continue;
		}
		for(j=0; j<framefilter_desc->frame.can_dlc; j++)
		{
//			printf(", %X", i, framefilter_desc->frame.data[j]);
			if(framefilter_desc->frame.data[j] != frame->data[j])
				break;
		}
		if(j != framefilter_desc->frame.can_dlc)
		{
//			printf("\n");
			continue;
		}
		
//		printf(", found\n");
		// detect same frame...
		ret = true;
		
		if(framefilter_remove(que, framefilter_desc) < 0)
			;	// failed
		else
			free(framefilter_desc);
		break;
	}
	pthread_mutex_unlock(&que->mutex);
	return ret;
}

