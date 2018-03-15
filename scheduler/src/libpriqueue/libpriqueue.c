/** @file libpriqueue.c
 */

#include <stdlib.h>
#include <stdio.h>

#include "libpriqueue.h"

/**
struct node_t
{
  void* value;
  struct node_t* next;
  struct node_t* parent;
}
*/


/**
  Initializes the priqueue_t data structure.

  Assumtions
    - You may assume this function will only be called once per instance of priqueue_t
    - You may assume this function will be the first function called using an instance of priqueue_t.
  @param q a pointer to an instance of the priqueue_t data structure
  @param comparer a function pointer that compares two elements.
  See also @ref comparer-page
 */
void priqueue_init(priqueue_t *q, int(*comparer)(const void *, const void *))
{
	q -> size = 0;
	q -> head = NULL;
	q -> tail = NULL;
	q -> compare = comparer;
}


/**
  Inserts the specified element into this priority queue.

  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr a pointer to the data to be inserted into the priority queue
  @return The zero-based index where ptr is stored in the priority queue, where 0 indicates that ptr was stored at the front of the priority queue.
 */
int priqueue_offer(priqueue_t *q, void *ptr)
{
	struct node_t* new_node = malloc(sizeof( struct node_t));
	new_node->value = ptr;
	new_node->parent = NULL;
	new_node->next = NULL;
	int index = 0; //location of insertion into queue, auto set to 0 for empty queue index  
	
	if (q->head == NULL)//if queue is empty
	{
		q->head = new_node;
		q->tail = new_node;
		q->size = q->size + 1;
	}
	else//if queue is not empty need to compare until find a proper place
	{
		struct node_t* current_node = q->head;
		int compare_value;//temp value to hold compare function return
		int search = 1;
		while (search == 1)//while still searching
		{			
			if (current_node == NULL)//if current node is null, reached end of list and must insert at end 
			{
				search = 0;//signal no longer searching for a place
			}
			else 
			{
				compare_value = q->compare(new_node->value, current_node->value);
				if (compare_value < 0)//if new_node value has higher priority and goes before current node
				{
					search = 0;//end the search
				}
				else//else keep searching
				{
					current_node = current_node->next;
					index = index + 1;					
				}
			}
		}
		
		if (current_node == NULL)//if no node found that is less priority add to back
		{
			q->tail->next = new_node;
			new_node->parent = q->tail;
			q->tail = new_node;
			q->size = q->size+1;
			index = index - 1;//need to go back one to deal with going past the last node
		}
		else if( index == 0)
		{
			q->head->parent = new_node;
			new_node->next = q->head;
			q->head = new_node;
			q->size = q->size +1;
		}
		else
		{
			struct node_t* previous = current_node->parent;
			previous->next = new_node;
			new_node->parent = previous;
			new_node->next = current_node;
			current_node->parent = new_node;
			q->size = q->size + 1;
		}
	}
	return (index);
}


/**
  Retrieves, but does not remove, the head of this queue, returning NULL if
  this queue is empty.

  @param q a pointer to an instance of the priqueue_t data structure
  @return pointer to element at the head of the queue
  @return NULL if the queue is empty
 */
void *priqueue_peek(priqueue_t *q)
{
	if (q -> head == NULL)
	{
		return NULL;
	}
	else
	{
		void* data = q -> head -> value;
		return data;
	}
}


/**
  Retrieves and removes the head of this queue, or NULL if this queue
  is empty.

  @param q a pointer to an instance of the priqueue_t data structure
  @return the head of this queue
  @return NULL if this queue is empty
 */
void *priqueue_poll(priqueue_t *q)
{
	void* data = q -> head -> value;
	if( q -> size == 0)
		{
			return NULL;
		}
	else if(q -> size == 1)
	{
		free (q -> head);
		q -> head = NULL:
		q -> size--;
		return data;
	}
	else
	{
		struct node_t* temp = q -> head;
		q -> head = q -> head -> next;
		free (q -> head);
		q -> size--;
		return data;
	}
}


/**
  Returns the element at the specified position in this list, or NULL if
  the queue does not contain an index'th element.

  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of retrieved element
  @return the index'th element in the queue
  @return NULL if the queue does not contain the index'th element
 */
void *priqueue_at(priqueue_t *q, int index)
{
	struct node_t* temp = q -> head;
	if(q -> size == 0)
	{
		return NULL;
	}
	else if (index >= q-> size || index < 0)
	{
		return NULL;
	}
	 else
		{
		for (int i = 0; i < index; i++)
		{
			temp = temp -> next;
		}
		return temp -> value;
		}
}


/**
  Removes all instances of ptr from the queue.

  This function should not use the comparer function, but check if the data contained in each element of the queue is equal (==) to ptr.

  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr address of element to be removed
  @return the number of entries removed
 */
int priqueue_remove(priqueue_t *q, void *ptr)
{
	int remove_count = 0;
	void *remove_value = ptr;
	if (p->size == 0)
	{
		return (0);
	}
	for (int i = 0; i < q->size; i++)
	{
		
	}
	
	return (remove_count);
}


/**
  Removes the specified index from the queue, moving later elements up
  a spot in the queue to fill the gap.

  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of element to be removed
  @return the element removed from the queue
  @return NULL if the specified index does not exist
 */
void *priqueue_remove_at(priqueue_t *q, int index)
{
	return 0;
}


/**
  Returns the number of elements in the queue.

  @param q a pointer to an instance of the priqueue_t data structure
  @return the number of elements in the queue
 */
int priqueue_size(priqueue_t *q)
{
	return q->size;
}


/**
  Destroys and frees all the memory associated with q.

  @param q a pointer to an instance of the priqueue_t data structure
 */
void priqueue_destroy(priqueue_t *q)
{
	for (int i = 0; i < q->size; i++)
	{
		priqueue_poll(q);
	}
}
