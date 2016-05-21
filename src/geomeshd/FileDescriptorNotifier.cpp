//
// Created by Werner Kroneman on 20-05-16.
//

#include "FileDescriptorNotifier.hpp"
#include "../Logger.hpp"

#include <assert.h>

using namespace std;

void FDNotifier::startListening() {
    stop = false;
    while (!stop)
    {
        waitForFD();
    }
}

void FDNotifier::addFileDescriptor(int fd, function<void (int fd)>
callback) {
    if (fd >= FD_SETSIZE)
        throw runtime_error("fd > FD_SETSIZE,"
                                    " please open an issue about this on GitHub.");

    listMux.lock();
    FD_SET(fd,&fds);
    listeners.insert(std::make_pair(fd,callback));
    listMux.unlock();
}

void FDNotifier::removeFileDescriptor(int fd) {
    listMux.lock();
    auto itr = listeners.find(fd);

    assert(itr != listeners.end());

    listeners.erase(itr);
    listMux.unlock();
}


void FDNotifier::waitForFD() {

    // Make a copy of the fd_set since
    // select overwrites it.
    listMux.lock();
    fd_set fds_read;
    memcpy((void*)&fds_read, (void*)&fds, sizeof(fd_set));
    listMux.unlock();

    // Add a 1/2 second timeout so that we can check for changes
    // in which fds to listen to or whether to terminate the thread.
    timeval timeout;
    timeout.tv_usec = 500000;
    timeout.tv_sec = 0;

    int res = select(FD_SETSIZE, &fds_read, nullptr,nullptr, &timeout);

    if (res < 0) {
        Logger::log(LogLevel::ERROR, strerror(errno));
    } else if (res > 0) {

        listMux.lock();
        for (int fd = 0; fd < FD_SETSIZE; ++fd) {
            if (FD_ISSET(fd, &fds_read)) {
                auto itr = listeners.find(fd);
                if (itr != listeners.end())
                    listeners[fd](fd);
            }
        }
        listMux.unlock();
    }
}

FDNotifier::~FDNotifier()
{
    stop = true;

    if (waiterThread.get() != nullptr) {
        waiterThread->join();
    }
}

FDNotifier::FDNotifier(bool async)
{

    memset((void*)&fds,0,sizeof(fds));

    if (async)
    {
        waiterThread.reset(new std::thread([&] {
            startListening();
        }));
    }
}