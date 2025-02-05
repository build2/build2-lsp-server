
module;

#if !defined(BUILD2_LSP_SERVER_ENABLE_IMPORT_STD)
#include <cstddef>
#include <optional>
#include <array>
#include <string_view>
#include <vector>
#include <ranges>
#include <regex>
#endif

export module lang.buildfile;

import lsp_boot;
import tracked_document_data;

#if defined(BUILD2_LSP_SERVER_ENABLE_IMPORT_STD)
import std;
#endif

namespace b2lsp
{
	export using namespace std::string_view_literals;

	export struct BuildfileDocument : TrackedDocumentData
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
				auto line = std::string_view(rg);
				LineTokenList tokens;

				// Very naive, minimal tokenization. Pending decision on parsing approach.

				std::optional< LineToken > trailing_comment;
				if (auto const hash = line.find('#'); hash != std::string_view::npos)
				{
					trailing_comment.emplace(hash, line.length() - hash, lsp_boot::lsp::SemanticTokenType::comment);
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
						tokens.emplace_back(base, str_literal.length(), lsp_boot::lsp::SemanticTokenType::string);
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
									tokens.emplace_back(base, mlen, lsp_boot::lsp::SemanticTokenType::keyword);
								}
								else if (std::ranges::contains(types, matched))
								{
									tokens.emplace_back(base, mlen, lsp_boot::lsp::SemanticTokenType::type);
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
