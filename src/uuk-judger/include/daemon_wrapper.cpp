#include "daemon.hpp"

Daemon *daemon_ptr = nullptr;

void sigchld_handler_wrapper(int sig)
{
    if (daemon_ptr)
        daemon_ptr->handle_sigchld();
}