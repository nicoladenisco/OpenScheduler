#include "ProxyApp.hpp"
#include <csignal>
#include <signal.h>
#include <stdio.h>

ProxyApp theApp;
struct sigaction actchld, oldchld, acthup, oldhup, actusr1, oldusr1, actbus,
    oldbus;

// gestore dei segnali demandato all'applicazione
void segnali(int segnale, siginfo_t *info, void *bo) {
  theApp.segnali(segnale, info, bo);
}

int main(int argc, char **argv) {
  puts("OpenScheduler slot engine - ver. " VERSION);
  puts("build: " __TIMESTAMP__);

  // gestione segnali
  actchld.sa_sigaction = segnali;
  sigemptyset(&actchld.sa_mask);
  actchld.sa_flags = SA_SIGINFO | SA_NOCLDSTOP;
  sigaction(SIGCHLD, &actchld, &oldchld);

  acthup.sa_sigaction = segnali;
  sigemptyset(&acthup.sa_mask);
  acthup.sa_flags = 0;
  sigaction(SIGHUP, &acthup, &oldhup);

  actusr1.sa_sigaction = segnali;
  sigemptyset(&actusr1.sa_mask);
  actusr1.sa_flags = 0;
  sigaction(SIGUSR1, &actusr1, &oldusr1);

  actbus.sa_sigaction = segnali;
  sigemptyset(&actbus.sa_mask);
  actbus.sa_flags = SA_SIGINFO | SA_NOCLDSTOP;
  sigaction(SIGBUS, &actbus, &oldbus);

  return theApp.main(argc, argv);
}
