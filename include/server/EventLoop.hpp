#pragma once 


#include <functional>
#include <unordered_map>
#include <sys/epoll.h>





class EventLoop {
    public:
        using Handler = std::function<void(int fd)>;

        EventLoop();
        
        ~EventLoop();

        void addFd(int fd, Handler handler);

        void removeFd(int Fd);

        void run();

        void stop();

    private:
        int epoll_fd;
        bool running_;
        std::unordered_map<int, Handler> handlers_;

        static const int MAX_EVENTS = 64;
};
