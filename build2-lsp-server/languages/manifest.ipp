
module;

#if defined(BUILD2_LSP_SERVER_ENABLE_IMPORT_STD)
import std;
#else
#include <string_view>
#include <ranges>
#endif

export module lang.manifest;

import lsp_boot;

namespace b2lsp
{
	using namespace std::string_view_literals;

	export struct ManifestImplementation
	{
		static constexpr auto supports_uri(std::string_view const uri) -> bool
		{
			return uri.ends_with("manifest"sv);
		}

		static auto handle(lsp_boot::lsp::requests::SemanticTokens const& msg) -> lsp_boot::Server::RequestResult
		{
			auto tokens = boost::json::array{};

			auto result = boost::json::object{
				// @todo: resultId for deltas: https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#semanticTokensLegend
				{ "data", tokens | std::ranges::to< boost::json::array >() },
			};
			return result;
		}
	};
}
