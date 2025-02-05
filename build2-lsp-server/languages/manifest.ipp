
module;

#if defined(BUILD2_LSP_SERVER_ENABLE_IMPORT_STD)
import std;
#else
#include <cstddef>
#include <tuple>
#include <string_view>
#include <vector>
#include <ranges>
#endif

export module lang.manifest;

import tracked_document_data;

import lsp_boot;
import lsp_boot.ext_mod_wrap.boost.json;

namespace b2lsp
{
	using namespace std::string_view_literals;

	export struct ManifestDocument : TrackedDocumentData
	{
		static constexpr auto supports_uri(std::string_view const uri) -> bool
		{
			return uri.ends_with("manifest"sv);
		}

		using TrackedDocumentData::TrackedDocumentData;

		auto handle(lsp_boot::lsp::requests::SemanticTokens const& msg) const -> lsp_boot::Server::RequestResult
		{
			struct LineToken
			{
				std::size_t line_char_offset;
				std::size_t length;
				lsp_boot::lsp::SemanticTokenType type;
			};

			using LineTokenList = std::vector< LineToken >;

			static constexpr auto line_tokens = [](std::ranges::contiguous_range auto&& rg) {
				auto const line = std::string_view(rg);
				LineTokenList tokens;
				// @todo: look into whether leading whitespace is allowed in manifests, either before comment or key
				if (line.starts_with('#'))
				{
					tokens.emplace_back(0, line.length(), lsp_boot::lsp::SemanticTokenType::comment);
				}
				else
				{
					auto const colon = line.find(':');
					if (colon != std::string_view::npos)
					{
						tokens.emplace_back(0, colon, lsp_boot::lsp::SemanticTokenType::keyword);
					}
					auto const semicolon = line.find(';'); // @todo: will be some complexity here with string literals in buildfile fragments
					if (semicolon != std::string_view::npos)
					{
						tokens.emplace_back(semicolon, line.length() - semicolon, lsp_boot::lsp::SemanticTokenType::comment);
					}
				}
				return tokens;
				};
			static constexpr auto to_tuples = [](auto const& indexed_tk_list) {
				auto const& [line, tokens] = indexed_tk_list;
				return tokens
					| std::views::transform([line](auto const& tk) { return std::tuple{ line, tk.line_char_offset, tk.length, tk.type }; })
					| std::ranges::to< std::vector >();
				};

			auto tokens = content()
				| std::views::split("\n"sv)
				| std::views::transform(line_tokens)
				| std::views::enumerate
				| std::views::transform(to_tuples)
				| std::views::join;

			auto result = boost::json::object{
				// @todo: resultId for deltas: https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#semanticTokensLegend
				{ "data", lsp_boot::lsp::generate_semantic_token_deltas(tokens | std::ranges::to< std::vector >()) | std::ranges::to< boost::json::array >() },
			};
			return result;
		}
	};
}
