#include "BENSCHILLIBOWL.h"

#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/shm.h>


bool IsEmpty(BENSCHILLIBOWL* bcb);
bool IsFull(BENSCHILLIBOWL* bcb);
void AddOrderToBack(Order **orders, Order *order);

MenuItem BENSCHILLIBOWLMenu[] = { 
    "BensChilli", 
    "BensHalfSmoke", 
    "BensHotDog", 
    "BensChilliCheeseFries", 
    "BensShake",
    "BensHotCakes",
    "BensCake",
    "BensHamburger",
    "BensVeggieBurger",
    "BensOnionRings",
};
int BENSCHILLIBOWLMenuLength = 10;

/* Select a random item from the Menu and return it */
MenuItem PickRandomMenuItem() {
    int item = rand()%BENSCHILLIBOWLMenuLength;
    return BENSCHILLIBOWLMenu[item];
}

/* Allocate memory for the Restaurant, then create the mutex and condition variables needed to instantiate the Restaurant */

BENSCHILLIBOWL* OpenRestaurant(int max_size, int expected_num_orders) {
    BENSCHILLIBOWL *bowl = (BENSCHILLIBOWL*)malloc(sizeof(BENSCHILLIBOWL));
    
    bowl->orders = NULL;
    // bowl->order_number = 0;
    bowl->current_size = 0;
    bowl->next_order_number = 1;
    bowl->orders_handled = 0;
    bowl->max_size = max_size;
    bowl->expected_num_orders = expected_num_orders;

    pthread_mutex_init(&(bowl->mutex), NULL);
    
    printf("Restaurant is open!\n");
    return bowl;
}


/* check that the number of orders received is equal to the number handled (ie.fullfilled). Remember to deallocate your resources */

void CloseRestaurant(BENSCHILLIBOWL* bcb) {
    if(bcb->orders_handled != bcb->expected_num_orders) {
        fprintf(stderr, "Some or all orders were not handled!\n");
        exit(0);
    }
    pthread_mutex_destroy(&(bcb->mutex));
    free(bcb);

    printf("Restaurant is closed!\n");
}

/* add an order to the back of queue */
int AddOrder(BENSCHILLIBOWL* bcb, Order* order) {
    pthread_mutex_lock(&(bcb->mutex));

    while (IsFull(bcb)) {
        pthread_cond_wait(&(bcb->can_add_orders), &(bcb->mutex));
    }

    if (bcb->orders == NULL) {
        bcb->orders = order;
        order->next = NULL;
    }
    else {
        AddOrderToBack(&(bcb->orders), order);
    }
    order->order_number = bcb->next_order_number;

    bcb->next_order_number++;
    bcb->current_size++;

    pthread_cond_broadcast(&(bcb->can_get_orders));
    pthread_mutex_unlock(&(bcb->mutex));

    return order->order_number;
}

/* remove an order from the queue */
Order *GetOrder(BENSCHILLIBOWL* bcb) {
    pthread_mutex_lock(&(bcb->mutex)); //  critical section

    while(IsEmpty(bcb)) {  
        if (bcb->orders_handled == bcb->expected_num_orders) {
            pthread_mutex_unlock(&(bcb->mutex));
            return NULL;
        }
        pthread_cond_wait(&(bcb->can_get_orders), &(bcb->mutex));
    }

    // Reached the front of the queue
    Order *my_order = bcb->orders;
    bcb->orders = my_order->next;
    bcb->current_size--; 
    bcb->orders_handled++;  
  
    pthread_cond_broadcast(&(bcb->can_add_orders));
    pthread_mutex_unlock(&(bcb->mutex));

    return my_order;
}

// Optional helper functions (you can implement if you think they would be useful)
bool IsEmpty(BENSCHILLIBOWL* bcb) {
  return (bcb->orders == NULL);
}

bool IsFull(BENSCHILLIBOWL* bcb) {
    int counter = 0;
    int max_size = bcb->max_size;
    Order* current_order = bcb->orders;

    while (current_order != NULL){
        counter+=1;
        current_order = current_order->next; 
    }
    return (max_size == counter);
}

/* this methods adds order to rear of queue */
void AddOrderToBack(Order **orders, Order *order) {
    Order *current_order = *orders;
    order->next = NULL;

    while (current_order->next != NULL) {
        current_order = current_order->next;
    }
    current_order->next = order;
}