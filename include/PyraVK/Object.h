#pragma once
#include <PyraVK/Essentials.h>


namespace pyra
{
	class Object
	{
	public:
		enum Type
		{
			UNDEFINED = 0,
			VULKAN_INSTANCE,
			WINDOW_SURFACE,
			RENDER_DEVICE,
			SWAPCHAIN,
			SHADER_MODULE,
			GRAPHICS_PIPELINE,
			VERTEX_BUFFER,
			INDEX_BUFFER
		};

		struct Config {};
	protected:
		const Type _type;
	public:
		Object(const Config config,const Type type) :  _type(type) {};
		virtual ~Object() = 0 {};

		Object(const Object&) = delete;
		const Object& operator=(const Object&) = delete;

		Object(Object&&) = default;
		Object& operator=(Object&&) = default;

		const Type getType() const { return _type; };
	};
}