#pragma once
#include <PyraVK/Essentials.h>


namespace pyra
{
	class PyraBase
	{
	public:
		PyraBase() = default;
		virtual ~PyraBase() = default;

		PyraBase(const PyraBase&) = delete;
		const PyraBase& operator=(const PyraBase&) = delete;

		PyraBase(PyraBase&&) = default;
		PyraBase& operator=(PyraBase&&) = default;
	};
}