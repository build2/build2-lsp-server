
module;

#include <boost/json/value_to.hpp>

#if !defined(BUILD2_LSP_SERVER_ENABLE_IMPORT_STD)
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <string_view>
#include <vector>
#include <ranges>
#endif

module lang.manifest;

import lang.common.semantic_tokens;
import utility;

import lsp_boot;
import lsp_boot.semantic_tokens;
import lsp_boot.ext_mod_wrap.boost.json;

#if defined(BUILD2_LSP_SERVER_ENABLE_IMPORT_STD)
import std;
#endif

namespace b2lsp
{
	using namespace std::string_view_literals;

	auto ManifestDocument::generate_semantic_tokens_for_range(lsp_boot::lsp::Range const range) const -> std::vector< unsigned int >
	{
		static constexpr auto line_tokens = [](auto const& line_tuple) {
			auto [line_index, line] = std::pair< std::size_t, std::string_view >{ line_tuple };
			LineTokenList tokens;
			// @todo: look into whether leading whitespace is allowed in manifests, either before comment or key
			if (line.starts_with('#'))
			{
				tokens.emplace_back(line_index, 0, line.length(), lsp_boot::lsp::SemanticTokenType::comment);
			}
			else
			{
				auto const colon = line.find(':');
				if (colon != std::string_view::npos)
				{
					tokens.emplace_back(line_index, 0, colon, lsp_boot::lsp::SemanticTokenType::keyword);
				}
				auto const semicolon = line.find(';'); // @todo: will be some complexity here with string literals in buildfile fragments
				if (semicolon != std::string_view::npos)
				{
					tokens.emplace_back(line_index, semicolon, line.length() - semicolon, lsp_boot::lsp::SemanticTokenType::comment);
				}
			}
			return tokens;
			};

		// @Note: Potentially returning a bit more than asked for since to simply we ignore character positions and just return all tokens between the first and last line inclusive.
		// LSP semantic token requests allow for returning additional tokens.
		auto const simple_range = lsp_boot::LineRange::from_start_and_end_inclusive(range.start.line, range.end.line);

		auto tokens = enumerated_lines_range(simple_range)
			| std::views::transform(line_tokens)
			| std::views::join;

		return lsp_boot::generate_semantic_token_deltas(tokens | std::ranges::to< std::vector >());
	}

	auto ManifestDocument::handle(lsp_boot::lsp::requests::SemanticTokensFull const& msg) const -> lsp_boot::Server::RequestResult
	{
		auto tokens = generate_semantic_tokens_for_range(entire_document_range());

		auto result = boost::json::object{
			// @todo: resultId for deltas: https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#semanticTokensLegend
			{ "data", tokens | std::ranges::to< boost::json::array >() },
		};
		return result;
	}

	auto ManifestDocument::handle(lsp_boot::lsp::requests::SemanticTokensRange const& msg) const -> lsp_boot::Server::RequestResult
	{
		auto&& params = msg.params();

		auto const range = lsp_boot::lsp::Range::from_json(params.at(lsp_boot::lsp::keys::range));
		auto tokens = generate_semantic_tokens_for_range(range);

		auto result = boost::json::object{
			// @todo: resultId for deltas: https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#semanticTokensLegend
			{ "data", tokens | std::ranges::to< boost::json::array >() },
		};
		return result;
	}
}
