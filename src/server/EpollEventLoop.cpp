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
    last_active_[fd] = time(nullptr);

    epoll_event ev{};
    ev.events  = EPOLLIN;
    ev.data.fd = fd;

    int op = (handlers_.count(fd) > 1) ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;

    if (epoll_ctl(epoll_fd, op, fd, &ev) < 0) {
        throw std::runtime_error("epoll_ctl failed for fd " + std::to_string(fd));
    }
}

void EventLoop::removeFd(int fd) {
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
    handlers_.erase(fd);
    last_active_.erase(fd);
}

void EventLoop::setServerFd(int fd) {
    server_fd_ = fd;
}

void EventLoop::sweepIdleConnections() {
    time_t now = time(nullptr);

    std::vector<int> to_close;

    for (auto& [fd, last_seen] : last_active_) {
        if (fd == server_fd_) continue;
        if (now - last_seen > IDLE_TIMEOUT_SECONDS) {
            to_close.push_back(fd);
        }
    }

    for (int fd : to_close) {
        std::cout << "[KEEPALIVE] idle timeout, closing fd " << fd << "\n";
        removeFd(fd);
        close(fd);
    }
}

void EventLoop::run() {
    running_ = true;

    epoll_event events[MAX_EVENTS];

    while (running_) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, EPOLL_WAIT_TIMEOUT_MS);

        if (nfds < 0) {
            if (!running_) break;
            std::cerr << "epoll_wait error\n";
            continue;
        }

        for (int i = 0; i < nfds; i++) {
            int fd = events[i].data.fd;

            last_active_[fd] = time(nullptr);

            auto it = handlers_.find(fd);
            if (it != handlers_.end()) {
                it->second(fd);
            }
        }

        sweepIdleConnections();
    }
}


void EventLoop::stop() {
    running_ = false;
}
