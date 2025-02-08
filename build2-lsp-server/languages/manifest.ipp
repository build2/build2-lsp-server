
module;

#if !defined(BUILD2_LSP_SERVER_ENABLE_IMPORT_STD)
#include <string_view>
#endif

export module lang.manifest;

import tracked_document_data;

import lsp_boot;

#if defined(BUILD2_LSP_SERVER_ENABLE_IMPORT_STD)
import std;
#endif

namespace b2lsp
{
	export struct ManifestDocument : TrackedDocumentData
	{
		static constexpr auto supports_uri(std::string_view const uri) -> bool
		{
			using namespace std::string_view_literals;

			return uri.ends_with("manifest"sv);
		}

		using TrackedDocumentData::TrackedDocumentData;

		auto handle(lsp_boot::lsp::requests::SemanticTokens const& msg) const -> lsp_boot::Server::RequestResult;
	};
}
