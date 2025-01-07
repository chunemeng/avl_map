#include "avl.h"

#include <gtest/gtest.h>
#include <random>

using alp::avl_map;


class AvlMapTest : public ::testing::Test {
protected:
    void SetUp() override {
        static_assert(sizeof(avl_map<int, int>) == 48);
    }

};


TEST(AvlMapTest, TestGGrandparentRotation) {
    avl_map<int, int> avl_map;
    avl_map.emplace(5, 1);
    avl_map.emplace(2, 2);
    avl_map.emplace(7, 3);
    avl_map.emplace(1, 4);
    avl_map.emplace(3, 5);
    avl_map.emplace(0, 6);
}

// worst case height of avl tree
double fun(int i) {

    double log2_value = std::log2(i + 2);

    double height = 1.44 * log2_value - 0.3277;

    int h_max = static_cast<int>(std::floor(height));

    return h_max;
}

TEST(AvlMapTest, RandomAdd) {
    std::map<int, int> vec;
    for (int i = 0; i < 100000; i++) {
        vec.emplace(std::rand(), std::rand());
    }
    std::vector<std::pair<int, int>> vec2(vec.begin(), vec.end());
    std::shuffle(vec2.begin(), vec2.end(), std::mt19937{std::random_device{}()});

    alp::avl_map<int, int> avl_map;
    int i = 0;
    for (auto it: vec2) {
        avl_map.emplace(it.first, it.second);
        if (i % 1000 == 0) {
            avl_map.check();
        }

        if (i % 1000 == 0) {
            EXPECT_LE(avl_map.height(), fun(i + 1));
        }

        auto it2 = avl_map.find(it.first);
        EXPECT_EQ(it2 != avl_map.end(), true);
        i++;

    }

    EXPECT_EQ(avl_map.size(), vec.size());
    auto it1 = vec.begin();
    auto it2 = avl_map.begin();

    for (; it1 != vec.end(); it1++, it2++) {
        EXPECT_EQ(it1->first, it2->first);
        EXPECT_EQ(it1->second, it2->second);
    }
}

TEST(AvlMapTest, RandomErase) {
    std::map<int, int> vec;
    alp::avl_map<int, int> avl_map;

    for (int i = 0; i < 100000; i++) {
        int a = i;
        vec.emplace(a, a);
        avl_map.emplace(a, a);
    }


    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10000; j++) {
            int a = rand() % 100000;
            vec.erase(a);
            avl_map.erase(a);
        }
        avl_map.check();
        EXPECT_EQ(avl_map.size(), vec.size());

        auto it1 = vec.begin();
        auto it2 = avl_map.begin();

        for (; it1 != vec.end(); it1++, it2++) {
            EXPECT_EQ(it2 != avl_map.end(), true);
            EXPECT_EQ(it1->first, it2->first);
            EXPECT_EQ(it1->second, it2->second);
        }
    }
}

TEST(AvlMapTest, RandomExtract) {
    std::map<int, int> vec;
    alp::avl_map<int, int> avl_map;

    for (int i = 0; i < 100000; i++) {
        int a = i;
        vec.emplace(a, a);
    }


    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10000; j++) {
            int a = rand() % 100000;
            auto node = vec.extract(a);
            if (!node.empty()) {
                avl_map.insert(avl_map.cbegin(), std::move(node));
            }

            EXPECT_EQ(avl_map.find(a) != avl_map.end(), true);
        }
        avl_map.check();
        EXPECT_EQ(avl_map.size(), 100000 - vec.size());
    }
}

TEST(AvlMapTest, EraseEmpty) {
    alp::avl_map<int, int> avl_map;
    avl_map.erase(1);
    avl_map.size();
    EXPECT_EQ(avl_map.size(), 0);
    EXPECT_EQ(avl_map.height(), 0);
    EXPECT_EQ(avl_map.begin() == avl_map.end(), true);
}

TEST(AvlMapTest, EraseRoot) {
    alp::avl_map<int, int> avl_map;
    avl_map.emplace(1, 1);
    avl_map.erase(1);
    EXPECT_EQ(avl_map.size(), 0);
    EXPECT_EQ(avl_map.height(), 0);
    EXPECT_EQ(avl_map.begin() == avl_map.end(), true);
}

using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;
using Microseconds = std::chrono::microseconds;

std::mt19937 rng(42);

template<template<typename, typename> class Map, int N = 10000, int Runs = 5>
void benchmark() {
    std::cout << "Benchmarking " << typeid(Map<int, int>).name() << " with N = " << N << ", Runs = " << Runs
              << std::endl;

    const int n = N;
    const int num = 2 * n;

    {
        Map<int, int> warmup_map;
        for (int i = 0; i < 1000; ++i) {
            warmup_map.emplace(i, i);
        }
    }

    std::vector<long> insert_times(Runs);
    for (int run = 0; run < Runs; ++run) {
        Map<int, int> avl_map;
        TimePoint start = Clock::now();
        for (int i = 0; i < n; ++i) {
            avl_map.emplace(i, i);
        }
        TimePoint end = Clock::now();
        insert_times[run] = std::chrono::duration_cast<Microseconds>(end - start).count();
    }

    double insert_avg = 0;
    for (auto t: insert_times) insert_avg += t;
    insert_avg /= Runs;
    double insert_var = 0;
    for (auto t: insert_times) insert_var += (t - insert_avg) * (t - insert_avg);
    insert_var /= Runs;
    double insert_stddev = std::sqrt(insert_var);

    std::cout << "Insert " << n << " elements:" << std::endl;
    std::cout << "    Avg Time: " << insert_avg << "us" << std::endl;
    std::cout << "    Std Dev: " << insert_stddev << "us" << std::endl;
    std::cout << "    Throughput: " << (double(n) * 1000000) / insert_avg << " ops/s" << std::endl;

    Map<int, int> avl_map;
    for (int i = 0; i < n; ++i) {
        avl_map.emplace(i, i);
    }

    std::vector<long> lookup_times(Runs);
    for (int run = 0; run < Runs; ++run) {
        TimePoint start = Clock::now();
        for (int i = 0; i < num; ++i) {
            auto it = avl_map.find(rng() % n);
            if (it == avl_map.end()) {
                std::cerr << "Error: Key not found!" << std::endl;
            }
        }
        TimePoint end = Clock::now();
        lookup_times[run] = std::chrono::duration_cast<Microseconds>(end - start).count();
    }

    double lookup_avg = 0;
    for (auto t: lookup_times) lookup_avg += t;
    lookup_avg /= Runs;
    double lookup_var = 0;
    for (auto t: lookup_times) lookup_var += (t - lookup_avg) * (t - lookup_avg);
    lookup_var /= Runs;
    double lookup_stddev = std::sqrt(lookup_var);

    std::cout << "Lookup " << num << " elements:" << std::endl;
    std::cout << "    Avg Time: " << lookup_avg << "us" << std::endl;
    std::cout << "    Std Dev: " << lookup_stddev << "us" << std::endl;
    std::cout << "    Throughput: " << (double(num) * 1000000) / lookup_avg << " ops/s" << std::endl;

    std::vector<long> erase_times(Runs);
    for (int run = 0; run < Runs; ++run) {
        Map<int, int> temp_map = avl_map;
        TimePoint start = Clock::now();
        for (int i = 0; i < num; ++i) {
            temp_map.erase(rng() % n);
        }
        TimePoint end = Clock::now();
        erase_times[run] = std::chrono::duration_cast<Microseconds>(end - start).count();
    }

    double erase_avg = 0;
    for (auto t: erase_times) erase_avg += t;
    erase_avg /= Runs;
    double erase_var = 0;
    for (auto t: erase_times) erase_var += (t - erase_avg) * (t - erase_avg);
    erase_var /= Runs;
    double erase_stddev = std::sqrt(erase_var);

    std::cout << "Erase " << num << " elements:" << std::endl;
    std::cout << "    Avg Time: " << erase_avg << "us" << std::endl;
    std::cout << "    Std Dev: " << erase_stddev << "us" << std::endl;
    std::cout << "    Throughput: " << (double(num) * 1000000) / erase_avg << " ops/s" << std::endl;

}
TEST(AvlMapTest, ThroughPut) {
    benchmark<std::map>();
    benchmark<alp::avl_map>();

}