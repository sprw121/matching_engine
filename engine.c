#include "engine.h"

#define NULL ((void *)0)

// Implements a O(n) solution to order matching
// While O(log(n)) solutions exist, for this simulated feed
// the naive solutions seems to work better due to cache efficency
// and reduced overhead.

// Representation of a limit order
// Size == 0 corresponds to a cancelled order
typedef struct orderContainer{
	t_size size;
	char trader[STRINGLEN];
	struct orderContainer* next;
} orderContainer;


// A pricepoint, represented as a queue of orders.
typedef struct priceQueue{
	orderContainer* first;
	orderContainer* last;
} priceQueue;

// An array to keep track of orders
// indexed by orderId, to allow for O(1) lookup.
static orderContainer orderList[MAX_TRADES+1];

// Array of pricepoints, each pricepoint being
// a queue of orders.
static priceQueue orderBook[MAX_PRICE+1];

static t_orderid currentOrder;
static int currentBid;
static int currentAsk;

// Helper functions

void copystring(char* d, char* t){
	d[0] = t[0];
	d[1] = t[1];
	d[2] = t[2];
	d[3] = t[3];
	d[4] = t[4];
}

//Sets the order size and trade
//Then places the order at the queue at that pricepoint
void queueOrder(t_order order){
	orderList[currentOrder].size = order.size;
	copystring(&(orderList[currentOrder].trader), &(order.trader));

	orderContainer* temp = (orderList + currentOrder);

	if(orderBook[order.price].first != NULL)
		(orderBook[order.price].last)->next = temp; 
	else
		orderBook[order.price].first = temp;
	
	orderBook[order.price].last = temp;
}

void findAsk(){
	while(currentAsk < MAX_PRICE && orderBook[currentAsk].first == NULL) currentAsk++ ;
}

void findBid(){
	while(currentBid > 0 && orderBook[currentBid].first == NULL) currentBid--;
}

//Sends execution reports to both parties in a trade
void trade(char* symbol, char* buyer, char* seller, t_size size, t_price price){
	t_execution executionReport;

	copystring(executionReport.symbol, symbol);
	executionReport.size = size;
	executionReport.price = price;

	executionReport.side = 0;
	copystring(executionReport.trader, buyer);
	execution(executionReport);

	executionReport.side = 1;
	copystring(executionReport.trader, seller);
	execution(executionReport);
}

// IMPLEMENTATION

void init() {
	memset(orderBook, 0, sizeof(priceQueue)*(MAX_PRICE+1));
	memset(orderList, 0, sizeof(orderContainer)*(MAX_TRADES+1));

	currentOrder = 0;
	currentBid = 0;
	currentAsk = MAX_PRICE;
}

void destroy() {
}

t_orderid limit(t_order order) {
	currentOrder++;

	priceQueue* pricePoint;
	orderContainer* nextFill;

	if(order.side == 0){
		while(order.price >= currentAsk){
			pricePoint = (orderBook + currentAsk);			
			nextFill = pricePoint->first;
		
			while(nextFill){
				if(nextFill->size < order.size){
					if(nextFill->size != 0){
						trade(order.symbol, order.trader,
						   nextFill->trader, nextFill->size, currentAsk);
						order.size -= nextFill->size;
					}

					nextFill = nextFill->next;
					pricePoint->first = nextFill;
				}
				else{	
					trade(order.symbol, order.trader,
						nextFill->trader, order.size, currentAsk);
					
					if(nextFill->size == 0)
						pricePoint->first = nextFill->next;
					else{
						nextFill->size -= order.size;
						pricePoint->first = nextFill;
					}
		
					return currentOrder;
				}
			}
				
			findAsk();
		}
	
		queueOrder(order);
		
		if(currentBid < order.price)
			currentBid = order.price;	
	}
	else{
		while(order.price <= currentBid){
			pricePoint = (orderBook + currentBid);
			nextFill = pricePoint->first;

			while(nextFill){
				if(nextFill->size < order.size){
					if(nextFill->size !=0){
						trade(order.symbol, nextFill->trader,
							order.trader, nextFill->size, currentBid);
						order.size -= nextFill->size;
					}
					
					nextFill = nextFill->next;
					pricePoint->first = nextFill;
				}
				else{
					trade(order.symbol, nextFill->trader,
						order.trader, order.size, currentBid);
		
					if(nextFill->size == order.size)
						pricePoint->first = nextFill->next;
					else{
						nextFill->size -= order.size;
						pricePoint->first = nextFill;
					}
					return currentOrder;
				}
			}

			findBid();
		}
	
		queueOrder(order);

		if(currentAsk > order.price)
			currentAsk = order.price;
	}

	return currentOrder;
}


// Cancel by setting order size to 0
// Skipping over canceled order implemented in limit()
void cancel(t_orderid orderid) {
	orderList[orderid].size = 0;
}

