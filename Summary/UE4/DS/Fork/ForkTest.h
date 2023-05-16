#ifndef FORKTEST_H
#define FORKTEST_H

#include <iostream>
#include <string>
#include <unistd.h>
#include <signal.h>
using namespace std;

void ParentChildProcessTest();

void SharedMemTest();

void SignalQueueTest();

void GameServerSignalTest();

#endif // FORKTEST_H