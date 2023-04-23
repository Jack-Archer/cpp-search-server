#pragma once


#include <algorithm>
#include <cstdlib>
#include <future>
#include <map>
#include <numeric>
#include <random>
#include <string>
#include <vector>


using namespace std::string_literals;

using namespace std;

template<typename Key, typename Value>
class ConcurrentMap {
public:
    using PairMapMutex = std::pair<std::map<Key, Value>, std::mutex>;
    
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys");
 
    struct Access {
        std::lock_guard<std::mutex> guard;
        Value &ref_to_value;

        Access(std::map<Key, Value> &m, const Key &key, std::mutex &mtx) : guard(std::lock_guard(mtx)), ref_to_value(m[key]) {}
    };
 
    explicit ConcurrentMap(size_t bucket_cnt) : array_count(bucket_cnt), data(new PairMapMutex[bucket_cnt]) {}
    
    void Erase(const Key& key) {
        size_t mapId = static_cast<uint64_t>(key) % array_count;
        data[mapId].first.erase(key);
    }
 
    Access operator[](const Key &key) {
        auto &[map, mtx] = data[(static_cast<uint64_t>(key) % array_count)];
        return Access(map, key, mtx);
    }
 
    std::map<Key, Value> BuildOrdinaryMap() {
        std::map<Key, Value> to_ret;
        for (size_t i = 0; i != array_count; i++) {
            std::lock_guard g(data[i].second);
            auto &chunk = data[i].first;
            to_ret.insert(chunk.begin(), chunk.end());
        }
        return to_ret;
    }
 
    ~ConcurrentMap() {
        delete[] data;
    }
 
private:
    size_t array_count;
    PairMapMutex *data;
    std::mutex AccessMutex;
    std::mutex BuildMutex;
};