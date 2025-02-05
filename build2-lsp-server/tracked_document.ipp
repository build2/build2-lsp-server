
module;

#if defined(BUILD2_LSP_SERVER_ENABLE_IMPORT_STD)
import std;
#else
#include <utility>
#include <string>
#include <string_view>
#include <tuple>
#include <variant>
#include <optional>
#endif

export module tracked_document;

import tracked_document_data;
import lang.manifest;
import lang.buildfile;

namespace b2lsp
{
	//export template < typename Impl >
	//struct TaggedDocument : TrackedDocumentData
	//{
	//	using TrackedDocumentData::TrackedDocumentData;
	//};

	//export using ManifestDocument = TaggedDocument< ManifestImplementation >;
	//export using BuildfileDocument = TaggedDocument< BuildfileImplementation >;

	export using TrackedDocument = std::variant<
		ManifestDocument,
		BuildfileDocument
	>;

	export auto create_tracked_document(std::string_view const uri, std::string content) -> std::optional< TrackedDocument >
	{
		using LangTuple = std::tuple<
			std::in_place_type_t< ManifestDocument >,
			std::in_place_type_t< BuildfileDocument >
			>;

		// Grab first matching implementation based on uri

		auto doc = std::optional< TrackedDocument >{};
		auto const select = [&]< typename Impl >(std::in_place_type_t< Impl >) -> bool {
			if (!doc && Impl::supports_uri(uri))
			{
				doc.emplace< Impl >(std::move(content));
				return true;
			}
			else
			{
				return false;
			}
		};
		std::apply([&]< typename... Impls >(Impls...) {
			(select(Impls{}) || ...);
			}, LangTuple{});
		return doc;
	}

	export auto update_document(TrackedDocument& doc, std::string new_content)
	{
		return std::visit([&]< typename Impl >(Impl& doc) {
			doc.update_content(std::move(new_content));
		}, doc);
	}

	export template < typename Operation >
	auto invoke_on_document(TrackedDocument const& doc, Operation&& op)
	{
		return std::visit([&]< typename Impl >(Impl const& doc) {
			return doc.handle(std::forward< Operation >(op));
		}, doc);
	}
}
