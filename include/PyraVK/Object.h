#pragma once
#include <PyraVK/Essentials.h>


namespace pyra
{
	class Object
	{
	public:
		Object() = default;
		virtual ~Object() = default;

		Object(const Object&) = delete;
		const Object& operator=(const Object&) = delete;

		Object(Object&&) = default;
		Object& operator=(Object&&) = default;
	};
}