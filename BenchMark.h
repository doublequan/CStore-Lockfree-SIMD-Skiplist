//
// Created by Quan Quan on 5/5/17.
//

#ifndef CLION_BENCHMARK_H
#define CLION_BENCHMARK_H

#include "CycleTimer.h"

#define THREAD_NUM 32

typedef void *(*func_t)(void *);


class BenchMark {
private:
public:

    BenchMark() {
    }

    double single_run(pthread_t *threads, int thread_num, func_t f, void *ptr) {
        double startTime = CycleTimer::currentSeconds();

        for (int i = 0; i < thread_num; i++) {
            pthread_create(&threads[i], NULL, f, ptr);
        }

        for (int i = 0; i < thread_num; i++) {
            pthread_join(threads[i], NULL);
        }

        double endTime = CycleTimer::currentSeconds();

        return endTime - startTime;
    }

    void run(int thread_num, func_t f, void *ptr) {
        pthread_t threads[thread_num];

        double time = single_run(threads, thread_num, f, ptr);

        printf("This run cost : %.3fs\n", time);
    }

    void run3x(int thread_num, func_t f, void *ptr) {
        pthread_t threads[thread_num];

        double time_sum = 0.0;
        for (int tmp = 0; tmp < 3; tmp++) {
            time_sum += single_run(threads, thread_num, f, ptr);
        }

        printf("This run cost : %.3fs\n", time_sum / 3);
    }

    void run10x(int thread_num, func_t f, void *ptr) {
        pthread_t threads[thread_num];

        double time_sum = 0.0;
        for (int tmp = 0; tmp < 10; tmp++) {
            time_sum += single_run(threads, thread_num, f, ptr);
        }

        printf("This run cost : %.3fs\n", time_sum / 10);
    }


    double single_mix_run(pthread_t *threads, int *thread_nums, func_t *f, void **ptrs, int num) {
        double startTime = CycleTimer::currentSeconds();

        int thread_count = 0;
        for (int n = 0; n < num; n++) {
            for (; thread_count < thread_nums[n]; thread_count++) {
                pthread_create(&threads[thread_count], NULL, f[n], ptrs[n]);
            }
        }

        thread_count = 0;
        for (int n = 0; n < num; n++) {
            for (; thread_count < thread_nums[n]; thread_count++) {
                pthread_join(threads[thread_count], NULL);
            }
        }

        double endTime = CycleTimer::currentSeconds();

        return endTime - startTime;
    }

    void mix_run(int *thread_nums, func_t *f, void **ptrs, int num, int run_times = 1) {
        int thread_num = 0;
        for (int i = 0; i < num; i++) thread_num += thread_nums[i];

        pthread_t threads[thread_num];

        double time_sum = 0.0;
        for (int tmp = 0; tmp < run_times; tmp++) {
            time_sum += single_mix_run(threads, thread_nums, f, ptrs, num);
        }
        printf("This run cost : %.3fs\n", time_sum / run_times);
    }


};


#endif //CLION_BENCHMARK_H
