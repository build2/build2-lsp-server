
module;

#if defined(BUILD2_LSP_SERVER_ENABLE_IMPORT_STD)
import std;
#else
#include <utility>
#include <string>
#include <string_view>
#include <vector>
#include <ranges>
#endif

export module tracked_document_data;

import utility;

import lsp_boot;
import lsp_boot.utility;

namespace b2lsp
{
	export struct TrackedDocumentData
	{
		static auto derive_lines(std::string_view const text)
		{
			using namespace std::string_view_literals;

			std::vector< std::string > lines;
			for (auto str = text; !str.empty(); )
			{
				auto const nl = str.find('\n');
				auto const line_end = nl == std::string_view::npos || nl == 0 || str[nl - 1] != '\r' ? nl : (nl - 1);
				lines.emplace_back(str.substr(0, line_end));
				str = nl == std::string_view::npos ? "" : str.substr(nl + 1);
			}
			return lines;
		}

		TrackedDocumentData(std::string content)
			: content_{ std::move(content) }
			, lines_{ derive_lines(content_) }
		{
		}

		auto content() const -> std::string_view
		{
			return content_;
		}

		auto lines() const -> std::span< std::string const >
		{
			return lines_;
		}

		auto lines_range(lsp_boot::LineRange const line_range) const
		{
			return lines_
				| std::views::drop(line_range.start())
				| std::views::take(line_range.size());
		}

		auto enumerated_lines_range(lsp_boot::LineRange const line_range) const
		{
			// @todo: pending libcpp enumerate
			return //lines_
				//| std::views::enumerate
				enumerate_workaround(lines_)
				| std::views::drop(line_range.start())
				| std::views::take(line_range.size());
		}

		auto entire_document_range() const
		{
			return lsp_boot::lsp::Range{
				lsp_boot::lsp::Location(0, 0),
				!lines().empty() ? lsp_boot::lsp::Location(lines().size() - 1, lines().back().size()) : lsp_boot::lsp::Location(0, 0),
			};
		}

		auto update_content(std::string new_content) -> void
		{
			content_ = std::move(new_content);
			lines_ = derive_lines(content_);
		}

		std::string content_;
		std::vector< std::string > lines_;
	};
}
