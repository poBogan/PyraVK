#pragma once 
#include <PyraVK/Object.h>

namespace pyra
{
	struct ResourceID
	{
		uint32_t generation;
		uint32_t index;
	};

	template<class T> struct Slot
	{
		std::optional<T> object;
		uint32_t generation;

		Slot<T>() noexcept = default;
		~Slot<T>() noexcept = default;

		Slot(const Slot<T>&) = delete;
		const Slot<T>& operator=(const Slot<T>&) = delete;

		Slot(Slot<T>&&) noexcept = default;
		Slot<T>& operator=(Slot<T>&&) noexcept = default;
	};

	template<class T> class ResourcePool : std::vector<Slot<T>>
	{
	private : 
		std::queue<uint32_t> emptySlots{};
	public : 
		ResourceID add_resource(T&& resource);
		bool resource_vaild(ResourceID id) const;
		const T& get_resource(ResourceID id) const;
		void destroy_resource(ResourceID id); 
		void clear_resources();
	};


	template<class T>
	inline ResourceID ResourcePool<T>::add_resource(T&& resource)
	{
		ResourceID id;
		if (emptySlots.empty())
		{
			Slot<T> newSlot;
			newSlot.generation = 1; 
			newSlot.object.emplace(std::move(resource));

			this->push_back(std::move(newSlot));
			id.generation = 1; 
			id.index = this->size() - 1;
		}

		else
		{
			id.index = emptySlots.front(); 
			emptySlots.pop(); 

			this->at(id.index).object.emplace(std::move(resource));
			id.generation = this->at(id.index).generation;
		}

		return id;
	}

	template<class T>
	inline bool ResourcePool<T>::resource_vaild(ResourceID id) const
	{
		if (id.index < this->size())
		{
			return (this->at(id.index).generation == id.generation && this->at(id.index).object.has_value());
		}

		else
		{
			return false;
		}
	}

	template<class T>
	inline const T& ResourcePool<T>::get_resource(ResourceID id) const
	{
		pyra_assert(resource_vaild(id));
		return this->at(id.index).object.value();
	}

	template<class T>
	inline void ResourcePool<T>::destroy_resource(ResourceID id)
	{
		if (resource_vaild(id))
		{
			this->at(id.index).object.reset();
			this->at(id.index).generation++; 
			emptySlots.push(id.index);
		}
		else
		{
			spdlog::error("");
		}
	}

	template<class T>
	inline void ResourcePool<T>::clear_resources()
	{
		for (int i = 0;i < this->size();i++)
		{
			Slot<T>& slot = this->at(i); 
			if (slot.object.has_value())
			{
				slot.object.reset(); 
				slot.generation++; 
				emptySlots.push(i);
			}
		}
	}

}