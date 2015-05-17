#include <deque>
#include <set>
#include <memory>
#include <string>
#include <exception>
#include <mutex>

using namespace std;

class Connection {
public:
    Connection() {};
    virtual ~Connection() {};
};

class ConnectionFactory {
public:
    ConnectionFactory() {};
    virtual ~ConnectionFactory() {};
    virtual shared_ptr<Connection> create()=0;
};

struct ConnectionPoolStats {
    size_t pool_size;
    size_t borrowed_size;
};

class ConnectionPoolException: public exception {
    public:
        ConnectionPoolException(const char* msg) : mMsg(msg) {}
        virtual ~ConnectionPoolException() throw() {}
        const char* what() const throw()  {return mMsg.c_str();}
    protected:
        string mMsg;
};

template <typename T>
class ConnectionPool {
    public:
        ConnectionPool(size_t pool_size, shared_ptr<ConnectionFactory> factory) {
            this->pool_size = pool_size;
            this->factory = factory;

            while (this->pool.size() < this->pool_size) {
                this->pool.push_back(this->factory->create());
            }
        }

        ConnectionPoolStats getStats() {
            ConnectionPoolStats stats;
            stats.pool_size = this->pool.size();
            stats.borrowed_size = this->borrowed.size();
            return stats;
        }

        shared_ptr<T> borrow() {
            lock_guard<mutex> lock(this->q_mutex);
            if (this->pool.size() == 0) {
                for (set<shared_ptr<Connection>>::iterator it = borrowed.begin();
                    it != borrowed.end(); ++it) {
                    try {
                        if ((*it).unique()) {
                            shared_ptr<Connection> conn = this->factory->create();
                            this->borrowed.erase(it);
                            this->borrowed.insert(conn);
                            return static_pointer_cast<T>(conn);
                        }
                    } catch ( const exception& e) {
                        throw ConnectionPoolException("No connections available");
                    }
                }

                throw ConnectionPoolException("No connections available");
            }

            auto conn = this->pool.front();
            this->borrowed.insert(conn);
            this->pool.pop_front();
            return static_pointer_cast<T>(conn);
        }

    void release(shared_ptr<T> conn) {
        lock_guard<mutex> lock(this->q_mutex);
        this->pool.push_back(static_pointer_cast<Connection> (conn));
        this->borrowed.erase(conn);
    }

    ~ConnectionPool() {};

    private:
        size_t pool_size;
        shared_ptr<ConnectionFactory> factory;
        deque<shared_ptr<Connection>> pool;
        set<shared_ptr<Connection>> borrowed;
        mutex q_mutex; 
};
