#pragma once
#include <PyraVK/PyraBase.h>
#include <shared_mutex>
#include <stack>

namespace pyra
{
    enum SlotState : uint8_t
    {
        empty,
        allocated,
        storing,
        stored
    };

    template<class T>
    struct Slot
    {
        std::optional<T> m_object;
        uint32_t m_generation = 1;
        std::atomic_uint8_t m_state = empty;
    };

    struct AssetID
    {
        uint32_t m_index = 0;
        uint32_t m_generation = 0;
    };

    template<class T, uint32_t block_size>
    class AssetPool
    {
        static_assert(block_size > 0, "block_size must be greater than 0");

        struct SlotLocation
        {
            SlotLocation() = default;
            explicit SlotLocation(const uint32_t global_slot_idx)
            {
                pool_index = global_slot_idx / block_size;
                block_index = global_slot_idx % block_size;
            }

            uint32_t pool_index;
            uint32_t block_index;
        };

        typedef std::array<Slot<T>,block_size> memory_block;

        std::vector<std::unique_ptr<memory_block>> m_pool;
        uint32_t m_next_slot_index = 0;
        std::stack<uint32_t,std::vector<uint32_t>> m_freed_slot_indices;
        std::shared_mutex m_pool_mutex;
    public :
        AssetID reserve_asset_id();
        void store_asset(AssetID reserved_id,T&& asset);
        bool asset_stored(AssetID id);
        T& get_asset(AssetID id);
        void destroy_asset(AssetID id);
    };

    template <class T, uint32_t block_size>
    AssetID AssetPool<T, block_size>::reserve_asset_id()
    {
        std::unique_lock lock(m_pool_mutex);
        AssetID id;
        if (m_freed_slot_indices.empty())
        {
            id.m_index = m_next_slot_index++;
            id.m_generation = 1;
            const SlotLocation slot_location(id.m_index);
            if (slot_location.pool_index == m_pool.size())
                m_pool.push_back(std::make_unique<memory_block>());

            auto& slot = m_pool.at(slot_location.pool_index)->at(slot_location.block_index);
            slot.m_generation = 1;
            slot.m_state.store(allocated,std::memory_order_release);
        }
        else
        {
            id.m_index = m_freed_slot_indices.top();
            m_freed_slot_indices.pop();
            const SlotLocation slot_location(id.m_index);
            auto& slot = m_pool.at(slot_location.pool_index)->at(slot_location.block_index);
            id.m_generation = slot.m_generation;
            slot.m_state.store(allocated,std::memory_order_release);
        }
        return id;
    }

    template <class T, uint32_t block_size>
    void AssetPool<T, block_size>::store_asset(AssetID reserved_id, T&& asset)
    {
        std::shared_lock lock(m_pool_mutex);

        SlotLocation slot_location(reserved_id.m_index);
        if (slot_location.pool_index >= m_pool.size())
        {
            spdlog::error("Cannot store asset! Invalid reserved asset id");
            return;
        }
        Slot<T>& slot = m_pool.at(slot_location.pool_index)->at(slot_location.block_index);
        if (slot.m_generation != reserved_id.m_generation)
        {
            spdlog::error("Cannot store asset! Invalid reserved asset id");
            return;
        }

        SlotState expected_state = allocated;
        if (slot.m_state.compare_exchange_strong(expected_state,storing,std::memory_order_acquire))
        {
            slot.m_object.emplace(std::move(asset));
            slot.m_state.store(stored, std::memory_order_release);
        }
        else
        {
            spdlog::error("Cannot store asset! Invalid reserved asset id");
        }
    }

    template <class T, uint32_t block_size>
    bool AssetPool<T, block_size>::asset_stored(AssetID id)
    {
        std::shared_lock lock(m_pool_mutex);

        SlotLocation slot_location(id.m_index);
        if (slot_location.pool_index < m_pool.size() == false)
            return false;

        Slot<T>& slot = m_pool.at(slot_location.pool_index)->at(slot_location.block_index);

        bool result = slot.m_generation == id.m_generation;
        result = result && slot.m_state.load(std::memory_order_acquire) == stored;
        return result;
    }

    template <class T, uint32_t block_size>
    T& AssetPool<T, block_size>::get_asset(const AssetID id)
    {
        std::shared_lock lock(m_pool_mutex);
        SlotLocation slot_location(id.m_index);

        if (slot_location.pool_index >= m_pool.size())
        {
            throw std::runtime_error("Cannot get asset! No such asset found");
        }

        Slot<T>& slot = m_pool.at(slot_location.pool_index)->at(slot_location.block_index);

        if (slot.m_generation != id.m_generation)
        {
            throw std::runtime_error("Cannot get asset! No such asset found");
        }

        if (slot.m_state.load(std::memory_order_acquire) != stored)
        {
            throw std::runtime_error("Cannot get asset! No such asset found");
        }

        return slot.m_object.value();
    }

    template <class T, uint32_t block_size>
    void AssetPool<T, block_size>::destroy_asset(AssetID id)
    {
        std::unique_lock lock(m_pool_mutex);
        SlotLocation slot_location(id.m_index);
        if (slot_location.pool_index >= m_pool.size())
        {
            spdlog::error("Cannot destroy asset! No such stored asset found");
            return;
        }

        Slot<T>& slot = m_pool.at(slot_location.pool_index)->at(slot_location.block_index);

        if (slot.m_generation != id.m_generation)
        {
            spdlog::error("Cannot destroy asset! No such stored asset found");
            return;
        }

        SlotState expected_state = stored;
        if (slot.m_state.compare_exchange_strong(expected_state,empty,std::memory_order_acquire))
        {
            slot.m_object.reset();
            ++slot.m_generation;
            m_freed_slot_indices.push(id.m_index);
        }
        else
        {
            spdlog::error("Cannot destroy asset! No such stored asset found");
        }
    }
}
