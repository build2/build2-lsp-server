
module;

#if defined(BUILD2_LSP_SERVER_ENABLE_IMPORT_STD)
import std;
#else
#include <string_view>
#endif

export module lang.buildfile;

import lsp_boot;

namespace b2lsp
{
	using namespace std::string_view_literals;

	export struct BuildfileImplementation
	{
		static constexpr auto supports_uri(std::string_view const uri) -> bool
		{
			// @todo: proper uri handling. currently just assume the ending is the file path.
			// see https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#uri
			return uri.ends_with("buildfile"sv)
				|| uri.ends_with("build2file"sv)
				|| uri.ends_with(".build"sv)
				|| uri.ends_with(".build2"sv);
		}

		static auto handle(lsp_boot::lsp::requests::SemanticTokens const& msg) -> lsp_boot::Server::RequestResult
		{
			return {};
		}
	};
}
