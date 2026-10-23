#pragma once
#include <PyraVK/Core.h>

namespace pyra
{
	class PyraBase
	{
	public:
		PyraBase() = default;
		~PyraBase() = default;

		PyraBase(const PyraBase&) = delete;
		void operator=(const PyraBase&) = delete;

		PyraBase(PyraBase&&) = default;
		PyraBase& operator=(PyraBase&&) = default;
	};
}