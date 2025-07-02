#include <iostream>
#include <fstream>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <sys/resource.h>

void signal_handler(int sig) {
    switch (sig) {
        case SIGUSR1:
            syslog(LOG_INFO, "SIGUSR1 отримано");
            break;
        case SIGINT:
            syslog(LOG_INFO, "SIGINT отримано, вимикаю Daemon");
            closelog();
            exit(0);
        default:
            break;
    }
}

void daemonize() {
    pid_t pid = fork();

    if (pid < 0) {
        std::cerr << "Fork вийшов з помилкою!" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        std::cout << "Daemon було запущено з PID = " << pid << std::endl;
        exit(EXIT_SUCCESS); // Завершення предка
    }

    // Створити нову сесію
    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }

    // Ігнорувати SIGHUP
    signal(SIGHUP, SIG_IGN);

    // Зміна поточної директорії
    if (chdir("/") < 0) {
        exit(EXIT_FAILURE);
    }

    // Закриття всіх відкритих файлових дескрипторів
    struct rlimit rl;
    getrlimit(RLIMIT_NOFILE, &rl);
    for (int fd = 0; fd < static_cast<int>(rl.rlim_max); fd++) {
        close(fd);
    }

    // Відкрити системний журнал
    openlog("cpp-daemon", LOG_PID, LOG_LOCAL0);
    syslog(LOG_INFO, "Daemon запущено успіншо!");

    // Обробники сигналів
    signal(SIGUSR1, signal_handler);
    signal(SIGINT, signal_handler);

    // Нескінченний цикл
    while (true) {
        pause(); // Очікування сигналу
    }
}

int main() {
    daemonize();
    return 0;
}

