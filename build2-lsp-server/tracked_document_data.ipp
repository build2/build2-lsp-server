
module;

#if defined(BUILD2_LSP_SERVER_ENABLE_IMPORT_STD)
import std;
#else
#include <utility>
#include <string>
#include <string_view>
#endif

export module tracked_document_data;

namespace b2lsp
{
	export struct TrackedDocumentData
	{
		TrackedDocumentData(std::string content) : content_{ std::move(content) }
		{
		}

		auto content() const -> std::string_view
		{
			return content_;
		}

		auto update_content(std::string new_content) -> void
		{
			content_ = std::move(new_content);
		}

		std::string content_;
	};
}
