#include <string>

#include "Rendering/VulkanRenderingEngine.hpp"
#include "Logging/Logging.hpp"

#define TEST_BUFFER_SIZE 0xffff

Engine::Rendering::VulkanRenderingEngine* rendering_engine;

static int boilerplate_rendering_engine_init()
{
	printf("boilerplate_rendering_engine_init start\n");
	rendering_engine = new Engine::Rendering::VulkanRenderingEngine();
	
	Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Info, "testlog");
	rendering_engine->init(300, 300, std::string("TESTING"));
	printf("boilerplate_rendering_engine_init end\n");

	return 0;
}

static int boilerplate_rendering_engine_cleanup()
{
	printf("boilerplate_rendering_engine_cleanup start\n");
	rendering_engine->cleanup();

	delete rendering_engine;
	printf("boilerplate_rendering_engine_cleanup end\n");
	
	return 0;
}

int test_vulkan_init_cleanup()
{
	assert(!boilerplate_rendering_engine_init());
	assert(!boilerplate_rendering_engine_cleanup());

	return 0;
}

int test_vulkan_draw_frame()
{
	assert(!boilerplate_rendering_engine_init());

	rendering_engine->drawFrame();

	assert(!boilerplate_rendering_engine_cleanup());

	return 0;
}

int test_vulkan_buffer_create_cleanup()
{
	assert(!boilerplate_rendering_engine_init());

	Engine::Rendering::VulkanBuffer* test_buffer = rendering_engine->createVulkanBuffer(TEST_BUFFER_SIZE, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	
	rendering_engine->cleanupVulkanBuffer(test_buffer);
	
	assert(!boilerplate_rendering_engine_cleanup());

	return 0;
}
