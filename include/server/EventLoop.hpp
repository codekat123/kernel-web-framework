#pragma once 


#include <functional>
#include <unordered_map>
#include <sys/epoll.h>
#include <ctime>




class EventLoop {
    public:
        using Handler = std::function<void(int fd)>;

        EventLoop();
        
        ~EventLoop();

        void addFd(int fd, Handler handler);

        void removeFd(int Fd);

        void run();

        void setServerFd(int fd);
        
        void stop();

    private:
        int epoll_fd;
        bool running_;
        int server_fd_ = -1;
        std::unordered_map<int, Handler> handlers_;
        std::unordered_map<int, time_t> last_active_;

        static const int MAX_EVENTS = 64;
        static const int IDLE_TIMEOUT_SECONDS = 30;
        static const int EPOLL_WAIT_TIMEOUT_MS = 1000;

        void sweepIdleConnections();
};
