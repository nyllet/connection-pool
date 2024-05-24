## Why did I fork this
This was a nice project to begin my learning of C++ 11.


## connection-pool

Generic, efficient, thread safe connection pooling for C++

### Introduction

We needed a safe, efficient way to concurrently access MySQL without connection banging.  

### Features

- Fast
- Thread safe
- Generic (MySQL implementation using Connection/C++ is provided)
- Lazy opening of connections
- Used connections can be returned for immediate reuse
- Unreturned connections needs to be reported so that they may be replaced
- Apache 2.0 license


### Example
```cpp
#include <string>
#include <memory>
#include "MySQLConnection.h"

// Create a pool of 5 MySQL connections
std::shared_ptr<MySQLConnectionFactory> mysql_connection_factory = std::make_shared<MySQLConnectionFactory>("mysql_server","mysql_username","mysql_password"));
std::shared_ptr<ConnectionPool<MySQLConnection> > mysql_pool = std::make_shared<ConnectionPool<MySQLConnection>>(5, mysql_connection_factory);


// Get a connection and do something with it (throws exception if pool is completely used)
std::unique_ptr<MySQLConnection> conn = mysql_pool->borrow();
// conn->sql_connection->do_whatever();

/* If our code dies here, the connection needs to be reported so that it can be replaced with a new one! :) */
mysql_pool->report_broken_connection();
// else release for immediate reuse
mysql_pool->return_connection(std::move(conn));

```

### Design notes
Connections are stored as a std::queue of std::unique_ptr so we can pop from the front and push from back; this makes sure all connections get cycled through as fast as possible (so we don't accidentally hang onto any dead ones for a long time).

We managed to get all of this WITHOUT a separate curator thread.  Calling borrow() should only block very briefly while we access the pool deque, etc.  If we have to replace a dead connection with a new one, borrow() will additionally block while the new connection is set up.  If we are still unable to serve a live connection, borrow() will hang while waiting.

The use of std::shared_ptr for tracking connections that was used previously in this project, has been replaced by std::unique_ptr since std::shared_ptr::unique() is deprecated. Actually, this was my main reason for cloning an refactoring.

Debugging may be enabled by using the ```#define _DEBUG(x)``` macro:

```cpp
// Log to stdout
#define _DEBUG(x) 								\
	do { 										\
		std::cout << "  (" << x << ")" << endl;	\
	} while(0)

```


### Dependencies

- Connection/C++ (for MySQL implementation)




