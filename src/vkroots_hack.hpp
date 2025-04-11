#pragma once

#include <unordered_map>
#include <memory>
#include <mutex>

namespace vkBasalt
{
// HACK:
// TODO: maybe find another way to iterate over swapchains
template <typename Key, typename Data>
class SynchronizedMapObjectExt {
public:
    using MapKey = Key;
    using MapData = SynchronizedMapObjectExt<Key, Data>;

    SynchronizedMapObjectExt(std::shared_ptr<Data> data)
      : m_data{std::move(data)} {
    }
    SynchronizedMapObjectExt(const SynchronizedMapObjectExt<Key, Data>& other)
      : m_data{other.m_data} {
    }
    SynchronizedMapObjectExt(SynchronizedMapObjectExt<Key, Data>&& other)
      : m_data{std::move(other.m_data)} {
    }
    SynchronizedMapObjectExt(std::nullptr_t) {
    }

    SynchronizedMapObjectExt& operator = (std::nullptr_t) {
      m_data = nullptr;
      return *this;
    }

    SynchronizedMapObjectExt& operator = (const SynchronizedMapObjectExt& other) {
      m_data = other.m_data;
      return *this;
    }

    SynchronizedMapObjectExt& operator = (SynchronizedMapObjectExt&& other) {
      m_data = std::move(other.m_data);
      return *this;
    }

    static SynchronizedMapObjectExt<Key, Data> get(const Key& key) {
      std::unique_lock lock{ s_mutex };
      auto iter = s_map.find(key);
      if (iter == s_map.end())
        return nullptr;
      return iter->second;
    }

    static SynchronizedMapObjectExt<Key, Data> create(const Key& key, Data data) {
      std::unique_lock lock{ s_mutex };
      auto val = s_map.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(key),
        std::forward_as_tuple(std::make_shared<Data>(std::move(data))));
      return val.first->second;
    }

    static bool remove(const Key& key) {
      std::unique_lock lock{ s_mutex };
      auto iter = s_map.find(key);
      if (iter == s_map.end())
        return false;
      s_map.erase(iter);
      return true;
    }

    Data* get() {
      return m_data.get();
    }

    const Data* get() const {
      return m_data.get();
    }

    Data* operator->() {
      return get();
    }

    const Data* operator->() const {
      return get();
    }

    bool has() const {
      return m_data != nullptr;
    }

    operator bool() const {
      return has();
    }

    const size_t count() const {
      return s_map.size();
    }

    static std::unordered_map<MapKey, MapData> raw() {
      return s_map;
    }

private:
    std::shared_ptr<Data> m_data;
    static std::mutex s_mutex;
    static std::unordered_map<MapKey, MapData> s_map;
};

#define VKROOTS_DEFINE_SYNCHRONIZED_MAP_TYPE_EXT(name, key) \
  using name = ::vkBasalt::SynchronizedMapObjectExt<key, name##Data>;

#define VKROOTS_IMPLEMENT_SYNCHRONIZED_MAP_TYPE_EXT(x) \
  template <> std::mutex x::s_mutex = {}; \
  template <> std::unordered_map<x::MapKey, x::MapData> x::s_map = {};

} // namespace vkBasalt
