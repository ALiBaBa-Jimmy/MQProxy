#pragma once
#include "publictype.h"
#include "xostype.h"

// Maximum connection number of the connection pool
#define MAX_CONNECT_NUM         100

// THe minumum connection number of the connection pool
#define MIN_CONNECT_NUM         2

// The increase number of connections when the cuurent connection is not enough to use.
#define INCREASE_CONNECT_NUM    1

// How many records will be fetched each time when execute query SQL
#define FETCH_BATCH_COUNT       10000


