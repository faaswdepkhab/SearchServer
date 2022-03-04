#include <cstdlib>
#include <future>
#include <map>
#include <numeric>
#include <random>
#include <string>
#include <vector>
#include <algorithm>

#include "log_duration.h"
//#include "test_runner_p.h"
const int DEFAULT_BUCKET_COUNT = 100;

using namespace std::string_literals;

template <typename Key, typename Value>
class ConcurrentMap {
public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys"s);

    struct Access {
    private:
        std::lock_guard<std::mutex> guard_;
    public:
        Value &ref_to_value;
        Access (std::pair<std::map<Key, Value>, std::mutex> &map, const Key &key) :
            guard_(map.second), ref_to_value(map.first[key]) {
            }
        };

    ConcurrentMap(size_t bucket_count = DEFAULT_BUCKET_COUNT) : map_pool_(bucket_count), bucket_count_(bucket_count) {};

    Access operator[](const Key &key) {
        size_t u_key = static_cast<size_t>(key);
        size_t bucket = u_key % bucket_count_;
        return Access(map_pool_[bucket], key);
    }
    
    Value at(const Key &key) const {
        size_t u_key = static_cast<size_t>(key);
        size_t bucket = u_key % bucket_count_;
        return map_pool_[bucket].first.at(key);
    }
    
    void erase(const Key &key) {
        size_t u_key = static_cast<size_t>(key);
        size_t bucket = u_key % bucket_count_;
        map_pool_[bucket].second.lock();
        map_pool_[bucket].first.erase(key);
        map_pool_[bucket].second.unlock();
    }
    
    std::map<Key, Value> BuildOrdinaryMap() {
        std::map<Key, Value> result;
        for(size_t bucket = 0; bucket < bucket_count_; ++bucket) {
            map_pool_[bucket].second.lock();
        }
        for(size_t bucket = 0; bucket < bucket_count_; ++bucket) {
            result.insert(map_pool_[bucket].first.begin(), map_pool_[bucket].first.end());
        }
        for(size_t bucket = 0; bucket < bucket_count_; ++bucket) {
            map_pool_[bucket].second.unlock();
        }
        return result;
    }
    
    std::map<Key, Value> BuildOrdinaryMapEmplace() {
        std::map<Key, Value> result;
        for(size_t bucket = 0; bucket < bucket_count_; ++bucket) {
            map_pool_[bucket].second.lock();
        }
        for(size_t bucket = 0; bucket < bucket_count_; ++bucket) {
            result.emplace(map_pool_[bucket].first.begin(), map_pool_[bucket].first.end());
        }
        for(size_t bucket = 0; bucket < bucket_count_; ++bucket) {
            map_pool_[bucket].second.unlock();
        }
        return result;
    }
    
    int count(const Key &key) const {
        size_t u_key = static_cast<size_t>(key);
        size_t bucket = u_key % bucket_count_;
        return map_pool_[bucket].first.count(key);
    }
        
    
private:
    std::mutex full_lock_;
    std::vector<std::pair<std::map<Key, Value>, std::mutex>> map_pool_;
    const size_t bucket_count_;
};
