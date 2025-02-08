
module;

#if !defined(BUILD2_LSP_SERVER_ENABLE_IMPORT_STD)
#include <string_view>
#include <vector>
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

		auto handle(lsp_boot::lsp::requests::SemanticTokensFull const& msg) const -> lsp_boot::Server::RequestResult;
		auto handle(lsp_boot::lsp::requests::SemanticTokensRange const& msg) const -> lsp_boot::Server::RequestResult;

	private:
		auto generate_semantic_tokens_for_range(lsp_boot::lsp::Range) const -> std::vector< unsigned int >;
	};
}
