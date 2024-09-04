#pragma once

#include "PlatformSemantics.hpp"

extern "C"
{
	CMEP_EXPORT void* CreateGeneratorData(
		void*		generator_script,
		const char* generator_script_fn,
		void*		generator_supplier,
		const char* generator_supplier_fn
	);
}
