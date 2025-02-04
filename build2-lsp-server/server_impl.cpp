
module;

#if defined(BUILD2_LSP_SERVER_ENABLE_IMPORT_STD)
import std;
#else
#endif

module server_impl;

namespace b2lsp
{
	ServerImplementation::~ServerImplementation() = default;

	auto ServerImplementation::pump() -> void
	{
	}
}
