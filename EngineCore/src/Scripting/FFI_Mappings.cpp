#include "Scripting/FFI_Mappings.hpp"

#include "Rendering/SupplyData.hpp"

#include "Scripting/ILuaScript.hpp"

#include <memory>

// Because we're in extern "C", use this to shorten names
using namespace Engine;

extern "C"
{
	CMEP_EXPORT void* CreateGeneratorData(
		void*		generator_script,
		const char* generator_script_fn,
		void*		generator_supplier,
		const char* generator_supplier_fn
	)
	{
		auto generator_script_cast = *static_cast<std::weak_ptr<Scripting::ILuaScript>*>(
			generator_script
		);
		auto generator_supplier_cast = *static_cast<std::weak_ptr<Scripting::ILuaScript>*>(
			generator_supplier
		);

		return new Rendering::GeneratorData{
			{generator_script_cast, generator_script_fn},
			{generator_supplier_cast, generator_supplier_fn}
		};
	}
}
