#pragma once
#include <PyraVK/PyraBase.h>
#include <shared_mutex>
#include <stack>

namespace pyra
{


    template<class T>
    struct Slot
    {
        std::optional<T> m_object;
        uint32_t m_generation = 1;
        std::atomic_bool m_locked = false;

        void lock();
        void unlock();
        bool try_lock();
    };

    template <class T>
    void Slot<T>::lock()
    {
        while (m_locked.exchange(true, std::memory_order_acquire))
        {
            m_locked.wait(true, std::memory_order_relaxed);
        }
    }

    template <class T>
    void Slot<T>::unlock()
    {
        m_locked.store(false, std::memory_order_release);
        m_locked.notify_one();
    }

    template <class T>
    bool Slot<T>::try_lock()
    {
        return !m_locked.exchange(true, std::memory_order_acquire);
    }

    struct AssetID
    {
        uint32_t m_slot_index = 0;
        uint32_t m_slot_generation = 0;
    };

    template <class T>
    struct LockedAsset
    {
        std::unique_lock<Slot<T>> container_slot_lock;
        T& asset;
    };

    template<class T, uint32_t block_size>
    class AssetPool
    {
        static_assert(block_size > 0, "block_size must be greater than 0");
        using block = std::array<Slot<T>, block_size>;
        std::vector<std::unique_ptr<block>> m_block_pointers;
        uint32_t m_next_slot_index = 0;
        std::stack<uint32_t> m_cleared_slot_indices;
        std::shared_mutex m_pool_mutex;
    public :
        AssetID store_asset(T&& asset);
        LockedAsset<T> get_asset(AssetID id);
        void destroy_asset(AssetID id);
    };

    template <class T, uint32_t block_size>
    AssetID AssetPool<T, block_size>::store_asset(T&& asset)
    {
        std::unique_lock pool_lock(m_pool_mutex);
        AssetID id;
        if (m_cleared_slot_indices.empty())
        {
            id.m_slot_index = m_next_slot_index++;
            id.m_slot_generation = 1;

            uint32_t pointer_idx = id.m_slot_index / block_size;
            uint32_t block_idx = id.m_slot_index % block_size;

            if (pointer_idx == m_block_pointers.size())
            {
                m_block_pointers.emplace_back(std::make_unique<block>());
            }

            Slot<T>& container_slot = (*m_block_pointers[pointer_idx])[block_idx];
            pool_lock.unlock();

            std::unique_lock slot_lock(container_slot);
            container_slot.m_generation = 1;
            container_slot.m_object.emplace(std::move(asset));
        }

        else
        {
            id.m_slot_index = m_cleared_slot_indices.top();
            m_cleared_slot_indices.pop();

            uint32_t pointer_idx = id.m_slot_index / block_size;
            uint32_t block_idx = id.m_slot_index % block_size;

            Slot<T>& container_slot = (*m_block_pointers[pointer_idx])[block_idx];
            pool_lock.unlock();

            std::unique_lock slot_lock(container_slot);
            id.m_slot_generation = container_slot.m_generation;
            container_slot.m_object.emplace(std::move(asset));
        }

        return id;
    }

    template <class T, uint32_t block_size>
    LockedAsset<T> AssetPool<T, block_size>::get_asset(AssetID id)
    {
        uint32_t pointer_idx = id.m_slot_index / block_size;
        uint32_t block_idx = id.m_slot_index % block_size;

        std::shared_lock pool_lock(m_pool_mutex);
        pyra_assert(pointer_idx < m_block_pointers.size());
        Slot<T>& container_slot = (*m_block_pointers[pointer_idx])[block_idx];
        pool_lock.unlock();

        std::unique_lock slot_lock(container_slot);
        pyra_assert(container_slot.m_object.has_value());
        pyra_assert(container_slot.m_generation == id.m_slot_generation);

        return LockedAsset<T>{.container_slot_lock = std::move(slot_lock), .asset = container_slot.m_object.value()};
    }

    template <class T, uint32_t block_size>
    void AssetPool<T, block_size>::destroy_asset(AssetID id)
    {
        uint32_t pointer_idx = id.m_slot_index / block_size;
        uint32_t block_idx = id.m_slot_index % block_size;

        std::shared_lock pool_lock(m_pool_mutex);
        pyra_assert(pointer_idx < m_block_pointers.size());
        Slot<T>& container_slot = (*m_block_pointers[pointer_idx])[block_idx];
        pool_lock.unlock();

        std::unique_lock slot_lock(container_slot);
        if (container_slot.m_generation == id.m_slot_generation && container_slot.m_object.has_value())
        {
            container_slot.m_object.reset();
            ++container_slot.m_generation;
            slot_lock.unlock();

            std::unique_lock write_pool_lock(m_pool_mutex);
            m_cleared_slot_indices.push(id.m_slot_index);
        }
    }
}
