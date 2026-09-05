#include <PyraVK/Renderer.h>

#include <GLFW/glfw3.h>

int main()
{
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(800, 800, "test program", 0, 0);

	uint32_t extentionCount;
	const char* const* requiredInstanceExtentions = glfwGetRequiredInstanceExtensions(&extentionCount);
	std::vector<const char*> instanceExtentionNames(requiredInstanceExtentions, requiredInstanceExtentions + extentionCount);

	pyra::Renderer::createRenderer(instanceExtentionNames);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}

	glfwDestroyWindow(window); 
	glfwTerminate();
}