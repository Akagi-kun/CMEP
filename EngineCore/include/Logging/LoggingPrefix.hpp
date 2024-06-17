#define LOGPFX_CLASS_LOGGER					 "[LOGGER ] "
#define LOGPFX_CLASS_ENGINE					 "[ENGINE ] "
#define LOGPFX_CLASS_VULKAN_RENDERING_ENGINE "[VRENDER] "
#define LOGPFX_CLASS_VULKAN_DEVICE_MANAGER	 "[VDEVMGR] "
#define LOGPFX_CLASS_VULKAN_IMAGE_FACTORY	 "[VIMG_F ] "
#define LOGPFX_CLASS_OBJECT					 "[OBJECT ] "
#define LOGPFX_CLASS_SCENE					 "[SCENE  ] "
#define LOGPFX_CLASS_SCENE_LOADER			 "[SCENE_L] "
#define LOGPFX_CLASS_SCENE_MANAGER			 "[SCENE_M] "
#define LOGPFX_CLASS_FONT					 "[FONT   ] "
#define LOGPFX_CLASS_FONT_FACTORY			 "[FONT_F ] "
#define LOGPFX_CLASS_TEXTURE				 "[TEXTURE] "
#define LOGPFX_CLASS_TEXTURE_FACTORY		 "[TXTR_F ] "
#define LOGPFX_CLASS_ASSET_MANAGER			 "[ASSET_M] "
#define LOGPFX_LUA_MAPPED					 "[LUA    ] "
#define LOGPFX_LUA_SCRIPT_EXECUTOR			 "[LUAEXEC] "

#define LOGPFX_NONE "[ ? ? ? ] "

#ifndef LOGPFX_CURRENT
#	define LOGPFX_CURRENT LOGPFX_NONE
#endif

// Include like this:
/*

// Prefixes for logging messages
#define LOGPFX_CURRENT __DESIRED_LOGPFX__
#include "LoggingPrefix.hpp"

 */
