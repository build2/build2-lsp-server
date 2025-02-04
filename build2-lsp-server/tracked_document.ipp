
module;

#if defined(BUILD2_LSP_SERVER_ENABLE_IMPORT_STD)
import std;
#else
#include <utility>
#include <string>
#include <tuple>
#include <variant>
#include <optional>
#endif

export module tracked_document;

import lang.manifest;
import lang.buildfile;

namespace b2lsp
{
	export struct TrackedDocumentData
	{
		TrackedDocumentData(std::string content) : content_{ std::move(content) }
		{
		}

		auto update_content(std::string new_content) -> void
		{
			content_ = std::move(new_content);
		}

		std::string content_;
	};

	export template < typename Impl >
	struct TaggedDocument : TrackedDocumentData
	{
		using TrackedDocumentData::TrackedDocumentData;
	};

	export using TrackedDocument = std::variant<
		TaggedDocument< ManifestImplementation >,
		TaggedDocument< BuildfileImplementation >
	>;

	export auto create_tracked_document(std::string_view const uri, std::string content) -> std::optional< TrackedDocument >
	{
		using LangTuple = std::tuple< ManifestImplementation, BuildfileImplementation >;

		// Grab first matching implementation based on uri

		auto doc = std::optional< TrackedDocument >{};
		auto const select = [&]< typename Impl >(std::in_place_type_t< Impl >) -> bool {
			if (!doc && Impl::supports_uri(uri))
			{
				doc.emplace< TaggedDocument< Impl > >(std::move(content));
				return true;
			}
			else
			{
				return false;
			}
		};
		std::apply([&]< typename... Impls >(Impls...) {
			(select(std::in_place_type< Impls >) || ...);
			}, LangTuple{});
		return doc;
	}

	export auto update_document(TrackedDocument& doc, std::string new_content)
	{
		return std::visit([&]< typename Impl >(TaggedDocument< Impl >& doc) {
			doc.update_content(std::move(new_content));
		}, doc);
	}

	export template < typename Operation >
	auto invoke_on_document(TrackedDocument const& doc, Operation&& op)
	{
		return std::visit([&]< typename Impl >(TaggedDocument< Impl > const& doc) {
			return Impl::handle(std::forward< Operation >(op));
		}, doc);
	}
}
