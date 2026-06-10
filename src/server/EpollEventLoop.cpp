#include <stdexcept>
#include <iostream>
#include <unistd.h>

#include "../../include/server/EventLoop.hpp"

EventLoop::EventLoop()
    : running_(false)
{
    epoll_fd = epoll_create1(0);

    if (epoll_fd < 0) {
        throw std::runtime_error("epoll_create1 failed");
    }
}

EventLoop::~EventLoop() {
    close(epoll_fd);
}

void EventLoop::addFd(int fd, Handler handler) {
    handlers_[fd] = handler;

    epoll_event ev{};
    ev.events  = EPOLLIN;
    ev.data.fd = fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) < 0) {
        throw std::runtime_error("epoll_ctl ADD failed for fd " + std::to_string(fd));
    }
}

void EventLoop::removeFd(int fd) {
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
    handlers_.erase(fd);
}

void EventLoop::run() {
    running_ = true;

    epoll_event events[MAX_EVENTS];

    while (running_) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

        if (nfds < 0) {
            if (!running_) break;
            std::cerr << "epoll_wait error\n";
            continue;
        }

        for (int i = 0; i < nfds; i++) {
            int fd = events[i].data.fd;

            auto it = handlers_.find(fd);
            if (it != handlers_.end()) {
                it->second(fd);
            }
        }
    }
}

void EventLoop::stop() {
    running_ = false;
}
