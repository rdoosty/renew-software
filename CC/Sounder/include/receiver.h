/*
 Copyright (c) 2018-2019, Rice University 
 RENEW OPEN SOURCE LICENSE: http://renew-wireless.org/license
 Author(s): Peiyao Zhao: pdszpy19930218@163.com 
            Rahman Doost-Mohamamdy: doost@rice.edu
 
----------------------------------------------------------
 Handles received samples from massive-mimo base station 
----------------------------------------------------------
*/

#ifndef DATARECEIVER_HEADER
#define DATARECEIVER_HEADER

#include "concurrentqueue.h"
#include "BaseRadioSet.h"
#include <algorithm>
#include <arpa/inet.h>
#include <cassert>
#include <chrono>
#include <ctime>
#include <iostream>
#include <netinet/in.h>
#include <numeric>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>

struct Event_data {
    int event_type;
    int data;
};

struct Package {
    uint32_t frame_id;
    uint32_t symbol_id;
    uint32_t cell_id;
    uint32_t ant_id;
    short data[];
    Package(int f, int s, int c, int a)
        : frame_id(f)
        , symbol_id(s)
        , cell_id(c)
        , ant_id(a)
    {
    }
};

struct SampleBuffer {
    std::vector<char> buffer;
    std::atomic_int* pkg_buf_inuse;
};

//std::atomic_int thread_count(0);
//std::mutex d_mutex;
//std::condition_variable cond;

class ClientRadioSet;

class Receiver {
public:
    // use for create pthread
    struct ReceiverContext {
        Receiver* ptr;
        SampleBuffer* buffer;
        int core_id;
        int tid;
    };

    struct dev_profile {
        int tid;
        int nsamps;
        int txSyms;
        int rxSyms;
        int txStartSym;
        unsigned txFrameDelta;
        double rate;
        std::string data_file;
        int core;
        Receiver* ptr;
    };

public:
    Receiver(int n_rx_threads, Config* config, moodycamel::ConcurrentQueue<Event_data>* in_queue);
    ~Receiver();

    std::vector<pthread_t> startRecvThreads(SampleBuffer* rx_buffer, unsigned in_core_id = 0);
    void completeRecvThreads(const std::vector<pthread_t>& recv_thread);
    std::vector<pthread_t> startClientThreads();
    void go();
    static void* loopRecv_launch(void* in_context);
    void loopRecv(ReceiverContext* context);
    static void* clientTxRx_launch(void* in_context);
    void clientTxRx(dev_profile* context);
    void getPathLoss();

private:
    Config* config_;
    ClientRadioSet* clientRadioSet_;
    BaseRadioSet* baseRadioSet_;

    int thread_num_;
    // pointer of message_queue_
    moodycamel::ConcurrentQueue<Event_data>* message_queue_;
};

#endif
