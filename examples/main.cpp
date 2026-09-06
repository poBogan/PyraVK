#include <PyraVK/Renderer.h>

#include <GLFW/glfw3.h>

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(800, 800, "test program", 0, 0);

	uint32_t extention_count = 0;
	const char** requiredExtentionsAddr = glfwGetRequiredInstanceExtensions(&extention_count);
	std::vector<const char*> requiredExtNames(requiredExtentionsAddr, requiredExtentionsAddr + extention_count);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}

	glfwDestroyWindow(window); 
	glfwTerminate();
}