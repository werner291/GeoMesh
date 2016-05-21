
#ifndef GEOMESH_FDNOTIFIER_H
#define GEOMESH_FDNOTIFIER_H

#include <functional>
#include <memory>
#include <map>
#include <mutex>
#include <sys/select.h>
#include <thread>

/**
 * Class responsible for keeping track of all
 * open file descriptors and sockets, and notify
 * their owners when data is available for reading.
 */
class FDNotifier {

    std::unique_ptr<std::thread> waiterThread;

    std::recursive_mutex listMux;
    std::map<int, std::function<void (int fd)> > listeners;

    volatile bool stop;

    volatile fd_set fds;

    /**
     * Run the listener thread. Called automatically
     * by constructor if async.
     * Note: this function will not return until the notifier
     * stops listenening.
     */
    void startListening();

    public:

    FDNotifier(bool async = true);

    void addFileDescriptor(int fd, std::function<void (int fd)> callback);

    void removeFileDescriptor(int fd);

    void waitForFD();

    ~FDNotifier();
    
};

#endif // GEOMESH_FDNOTIFIER_H

