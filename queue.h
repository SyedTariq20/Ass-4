#include <stdio.h>
#include <stdlib.h>
int item;
struct Node {
	int data;
	struct Node *next;
};
 
struct Queue {
	struct Node *front;
	struct Node *last;
	unsigned int size;
};
 
void init(struct Queue *q) {
	q->front = NULL;
	q->last = NULL;
	q->size = 0;
}
 

int pop(struct Queue *q) {
	if(q->size == 0)
    {
        return -1;
    }
	item = q->front->data;
	q->size--;
    
	struct Node *tmp = q->front;
	q->front = q->front->next;
	free(tmp);
	return item;
}
 
void push(struct Queue *q, int data) {
	q->size++;
 
	if (q->front == NULL) {
		q->front = (struct Node *) malloc(sizeof(struct Node));
		q->front->data = data;
		q->front->next = NULL;
		q->last = q->front;
	} else {
		q->last->next = (struct Node *) malloc(sizeof(struct Node));
		q->last->next->data = data;
		q->last->next->next = NULL;
		q->last = q->last->next;
	}
}
 

