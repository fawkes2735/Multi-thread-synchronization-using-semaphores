#include "semaphore_class.h"
#include <sstream>

const int NUM_READERS = 5;
const int NUM_WRITERS = 5;
const int NUM_PHIL    = 5;
const int ITER        = 5;

pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;

void ts_print(const std::string &msg) {
    pthread_mutex_lock(&print_lock);
    std::cout << msg << std::endl;
    pthread_mutex_unlock(&print_lock);
}

/*PROBLEM 1 – No-starve Readers–Writers*/

Semaphore ns_turnstile(1);
Semaphore ns_roomEmpty(1);
Semaphore ns_readMutex(1);
int ns_readCount = 0;

void *ns_reader(void *threadID) {
    int id = (long)threadID;

    for (int i = 0; i < ITER; i++) {

        ns_turnstile.wait();
        ns_turnstile.signal();

        ns_readMutex.wait();
        ns_readCount++;
        if (ns_readCount == 1)
            ns_roomEmpty.wait();
        ns_readMutex.signal();

        ts_print("[Problem 1] Reader " + std::to_string(id) + " READING");
        usleep(50000);

        ns_readMutex.wait();
        ns_readCount--;
        if (ns_readCount == 0)
            ns_roomEmpty.signal();
        ns_readMutex.signal();

        usleep(30000);
    }
    pthread_exit(NULL);
}

void *ns_writer(void *threadID) {
    int id = (long)threadID;

    for (int i = 0; i < ITER; i++) {
        ns_turnstile.wait();
        ns_roomEmpty.wait();

        ts_print("[Problem 1] Writer " + std::to_string(id) + " WRITING");
        usleep(70000);

        ns_roomEmpty.signal();
        ns_turnstile.signal();

        usleep(30000);
    }
    pthread_exit(NULL);
}

void run_problem1() {
    ts_print("Running Problem 1: No-starve Readers–Writers");
    pthread_t readers[NUM_READERS];
    pthread_t writers[NUM_WRITERS];

    for (long i = 0; i < NUM_READERS; i++)
        pthread_create(&readers[i], NULL, ns_reader, (void*)i);

    for (long i = 0; i < NUM_WRITERS; i++)
        pthread_create(&writers[i], NULL, ns_writer, (void*)i);

    for (int i = 0; i < NUM_READERS; i++) pthread_join(readers[i], NULL);
    for (int i = 0; i < NUM_WRITERS; i++) pthread_join(writers[i], NULL);
}

/*PROBLEM 2 – Writer-priority Readers–Writers*/

Semaphore wp_rmutex(1);
Semaphore wp_wmutex(1);
Semaphore wp_readTry(1);
Semaphore wp_resource(1);

int wp_rcount = 0;
int wp_wcount = 0;

void *wp_reader(void *threadID) {
    int id = (long)threadID;

    for (int i = 0; i < ITER; i++) {

        wp_readTry.wait();

        wp_rmutex.wait();
        wp_rcount++;
        if (wp_rcount == 1)
            wp_resource.wait();
        wp_rmutex.signal();
        wp_readTry.signal();

        ts_print("[Problem 2] Reader " + std::to_string(id) + " READING");
        usleep(50000);

        wp_rmutex.wait();
        wp_rcount--;
        if (wp_rcount == 0)
            wp_resource.signal();
        wp_rmutex.signal();

        usleep(30000);
    }
    pthread_exit(NULL);
}


void *wp_writer(void *threadID) {
    int id = (long)threadID;

    for (int i = 0; i < ITER; i++) {

        wp_wmutex.wait();
        wp_wcount++;
        if (wp_wcount == 1)
            wp_readTry.wait();
        wp_wmutex.signal();

        wp_resource.wait();
        ts_print("[Problem 2] Writer " + std::to_string(id) + " WRITING");
        usleep(70000);
        wp_resource.signal();

        wp_wmutex.wait();
        wp_wcount--;
        if (wp_wcount == 0)
            wp_readTry.signal();
        wp_wmutex.signal();

        usleep(30000);
    }
    pthread_exit(NULL);
}



void run_problem2() {
    ts_print("Running Problem 2: Writer-priority Readers–Writers");
    pthread_t readers[NUM_READERS];
    pthread_t writers[NUM_WRITERS];

    for (long i = 0; i < NUM_READERS; i++)
        pthread_create(&readers[i], NULL, wp_reader, (void*)i);

    for (long i = 0; i < NUM_WRITERS; i++)
        pthread_create(&writers[i], NULL, wp_writer, (void*)i);

    for (int i = 0; i < NUM_READERS; i++) pthread_join(readers[i], NULL);
    for (int i = 0; i < NUM_WRITERS; i++) pthread_join(writers[i], NULL);
}

/*PROBLEM 3 – Dining Philosophers #1 (Footman)*/

Semaphore dp1_fork[NUM_PHIL] = {
    Semaphore(1), Semaphore(1), Semaphore(1),
    Semaphore(1), Semaphore(1)
};
Semaphore dp1_footman(4);

void *phil1(void *threadID) {
    int id = (long)threadID;

    for (int i = 0; i < ITER; i++) {
        ts_print("[Problem 3] Philosopher " + std::to_string(id) + " THINKING");
        usleep(60000);

        dp1_footman.wait();
        dp1_fork[id].wait();
        dp1_fork[(id+1)%NUM_PHIL].wait();

        ts_print("[Problem 3] Philosopher " + std::to_string(id) + " EATING");
        usleep(70000);

        dp1_fork[id].signal();
        dp1_fork[(id+1)%NUM_PHIL].signal();
        dp1_footman.signal();
    }
    pthread_exit(NULL);
}

void run_problem3() {
    ts_print("Running Problem 3: Dining Philosophers #1 (Footman)");
    pthread_t p[NUM_PHIL];
    for (long i = 0; i < NUM_PHIL; i++)
        pthread_create(&p[i], NULL, phil1, (void*)i);
    for (int i = 0; i < NUM_PHIL; i++)
        pthread_join(p[i], NULL);
}

/* PROBLEM 4 – Dining Philosophers #2 (Asymmetric)*/

Semaphore dp2_fork[NUM_PHIL] = {
    Semaphore(1), Semaphore(1), Semaphore(1),
    Semaphore(1), Semaphore(1)
};

void *phil2(void *threadID) {
    int id = (long)threadID;

    for (int i = 0; i < ITER; i++) {
        ts_print("[Problem 4] Philosopher " + std::to_string(id) + " THINKING");
        usleep(60000);

        if (id == 0) {
            dp2_fork[(id+1)%NUM_PHIL].wait();
            dp2_fork[id].wait();
        } else {
            dp2_fork[id].wait();
            dp2_fork[(id+1)%NUM_PHIL].wait();
        }

        ts_print("[Problem 4] Philosopher " + std::to_string(id) + " EATING");
        usleep(70000);

        dp2_fork[id].signal();
        dp2_fork[(id+1)%NUM_PHIL].signal();
    }
    pthread_exit(NULL);
}

void run_problem4() {
    ts_print("Running Problem 4: Dining Philosophers #2 (Asymmetric)");
    pthread_t p[NUM_PHIL];
    for (long i = 0; i < NUM_PHIL; i++)
        pthread_create(&p[i], NULL, phil2, (void*)i);
    for (int i = 0; i < NUM_PHIL; i++)
        pthread_join(p[i], NULL);
}

/* main – select which problem to run*/

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cout << "Usage: ./cse4001_sync <problem#>\n";
        std::cout << " 1 = No-starve Readers–Writers\n";
        std::cout << " 2 = Writer-priority Readers–Writers\n";
        std::cout << " 3 = Dining Philosophers #1 (Footman)\n";
        std::cout << " 4 = Dining Philosophers #2 (Asymmetric)\n";
        return 1;
    }

    int p = atoi(argv[1]);

    switch (p) {
        case 1: run_problem1(); break;
        case 2: run_problem2(); break;
        case 3: run_problem3(); break;
        case 4: run_problem4(); break;
        default:
            std::cout << "Invalid problem number\n";
            return 1;
    }

    std::cout << "Main: all threads finished. Exiting.\n";
    pthread_exit(NULL);
}
