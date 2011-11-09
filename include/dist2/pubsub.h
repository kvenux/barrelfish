#ifndef PUBSUB_H_
#define PUBSUB_H_

#define MAX_SUBSCRIPTIONS 255

#include <stdint.h>

typedef uint64_t subscription_t;
typedef void(*subscription_handler_fn)(subscription_t id, char* object);

errval_t dist_subscribe(subscription_handler_fn, subscription_t*, char*, ...);
errval_t dist_unsubscribe(subscription_t);
errval_t dist_publish(char*, ...);

#endif /* PUBSUB_H_ */
