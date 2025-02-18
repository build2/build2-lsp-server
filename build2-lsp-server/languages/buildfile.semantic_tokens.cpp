
module;

#include <boost/json/value_to.hpp>

#if !defined(BUILD2_LSP_SERVER_ENABLE_IMPORT_STD)
#include <cstddef>
#include <cstdint>
#include <optional>
#include <array>
#include <string_view>
#include <vector>
#include <ranges>
#include <algorithm>
#include <regex>
#endif

module lang.buildfile;

import lang.common.semantic_tokens;
import utility;

import lsp_boot;
import lsp_boot.utility;
import lsp_boot.semantic_tokens;
import lsp_boot.ext_mod_wrap.boost.json;

#if defined(BUILD2_LSP_SERVER_ENABLE_IMPORT_STD)
import std;
#endif

namespace b2lsp
{
	using namespace std::string_view_literals;

	auto BuildfileDocument::generate_semantic_tokens_for_range(lsp_boot::lsp::Range const range) const -> std::vector< unsigned int >
	{
		static constexpr auto line_tokens = [](auto const& line_tuple) {
			auto [line_index, line] = std::pair< std::size_t, std::string_view >{ line_tuple };
			LineTokenList tokens;

			// Very naive, minimal tokenization. Pending decision on parsing approach.

			std::optional< LineToken > trailing_comment;
			if (auto const hash = line.find('#'); hash != std::string_view::npos)
			{
				trailing_comment.emplace(line_index, hash, line.length() - hash, lsp_boot::lsp::SemanticTokenType::comment);
				line = line.substr(0, hash);
			}

			static constexpr auto rx_str_word = "[a-zA-Z_][a-zA-Z0-9_.]*"sv;
			static auto const rx_word = std::regex(std::begin(rx_str_word), std::end(rx_str_word));

			constexpr auto keywords = std::array{
				"if"sv,
				"elif"sv,
				"else"sv,
				"switch"sv,
				"case"sv,
				"default"sv,
				"for"sv,
				"true"sv,
				"false"sv,
				"config"sv,
				"using"sv,

				// are directives meaningfully distinct?
				"define"sv,
				"include"sv,
				"source"sv,
				"import"sv,
				"info"sv,
				"text"sv,
				"warn"sv,
				"fail"sv,
				"assert"sv,
				"print"sv,
				"dump"sv,
				"diag"sv,
			};

			constexpr auto types = std::array{
				"bool"sv,
				"int64"sv,
				"int64s"sv,
				"uint64"sv,
				"uint64s"sv,
				"string"sv,
				"strings"sv,
				"string_set"sv,
				"string_map"sv,
				"path"sv,
				"paths"sv,
				"dir_path"sv,
				"dir_paths"sv,
				"json"sv,
				"json_array"sv,
				"json_object"sv,
				"json_set"sv,
				"json_map"sv,
				"name"sv,
				"names"sv,
				"name_pair"sv,
				"cmdline"sv,
				"project_name"sv,
				"target_triple"sv,
			};

			static constexpr auto if_quote = [](char const c) -> std::optional< char > {
				if (c == '\'' || c == '"')
				{
					return c;
				}
				else
				{
					return std::nullopt;
				}
				};

			std::size_t base = 0;
			while (line.length() > 0)
			{
				if (auto quote = if_quote(line[0]); quote.has_value())
				{
					auto const end = line.find(*quote, 1); // @note: doesn't handle escaping
					auto const str_literal = line.substr(0, end == std::string_view::npos ? end : (end + 1));
					tokens.emplace_back(line_index, base, str_literal.length(), lsp_boot::lsp::SemanticTokenType::string);
					line = line.substr(str_literal.length());
					base += str_literal.length();
				}
				else
				{
					std::match_results< std::string_view::iterator > m;
					if (std::regex_search(std::begin(line), std::end(line), m, rx_word, std::regex_constants::match_continuous))
					{
						auto const matched = line.substr(0, m.length());
						auto const mlen = matched.length();
						if (mlen == line.length() || !"./\\"sv.contains(line[mlen]))
						{
							if (std::ranges::contains(keywords, matched))
							{
								tokens.emplace_back(line_index, base, mlen, lsp_boot::lsp::SemanticTokenType::keyword);
							}
							else if (std::ranges::contains(types, matched))
							{
								tokens.emplace_back(line_index, base, mlen, lsp_boot::lsp::SemanticTokenType::type);
							}
						}

						line = line.substr(mlen);
						base += mlen;
					}
					else
					{
						line = line.substr(1);
						base += 1;
					}
				}
			}

			if (trailing_comment.has_value())
			{
				tokens.push_back(*trailing_comment);
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

	auto BuildfileDocument::handle(lsp_boot::lsp::requests::SemanticTokensFull const& msg) const -> lsp_boot::Server::RequestResult
	{
		// @todo: work out why VS specifically seems to send textDocument/semanticTokens/full endlessly with no interaction.

		auto tokens = generate_semantic_tokens_for_range(entire_document_range());

		auto result = boost::json::object{
			// @todo: resultId for deltas: https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#semanticTokensLegend
			{ "data", tokens | std::ranges::to< boost::json::array >() },
		};
		return lsp_boot::Server::RequestSuccessResult{
			result
		};
	}

	auto BuildfileDocument::handle(lsp_boot::lsp::requests::SemanticTokensRange const& msg) const -> lsp_boot::Server::RequestResult
	{
		auto&& params = msg.params();

		auto const range = lsp_boot::lsp::Range::from_json(params.at(lsp_boot::lsp::keys::range));
		auto tokens = generate_semantic_tokens_for_range(range);

		auto result = boost::json::object{
			// @todo: resultId for deltas: https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#semanticTokensLegend
			{ "data", tokens | std::ranges::to< boost::json::array >() },
		};
		return lsp_boot::Server::RequestSuccessResult{
			result
		};
	}
}
