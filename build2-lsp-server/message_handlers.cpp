
module;

// workaround: MSVC modules template specialization and boost::system::error_code
#if defined(_MSC_VER) && !defined(__clang__)
#include <boost/json/value_to.hpp>
#endif

#if defined(BUILD2_LSP_SERVER_ENABLE_IMPORT_STD)
import std;
#else
#include <cstddef>
#include <utility>
#include <tuple>
#include <optional>
#include <variant>
#include <expected>
#include <string>
#include <span>
#include <ranges>
#include <memory>
#endif

module server_impl;

import lsp_boot.ext_mod_wrap.boost.json;

namespace b2lsp
{
	using namespace lsp_boot;
	namespace requests = lsp::requests;
	namespace notifications = lsp::notifications;

	auto ServerImplementation::operator() (requests::Initialize&& msg) -> RequestResult
	{
		// @todo: check client capabilities, e.g. textDocument.publishDiagnostics

		auto semantic_tokens_legend = boost::json::object{
			{ "tokenTypes", boost::json::array(std::begin(lsp::semantic_token_types), std::end(lsp::semantic_token_types)) },
			{ "tokenModifiers", boost::json::array() },
		};

		auto result = boost::json::object{
			{ lsp::keys::capabilities, {
				{ "textDocumentSync", 1 }, // @todo: incremental support
				{ "semanticTokensProvider", {
					{ "legend", semantic_tokens_legend },
					{ "range", true },
					{ "full", true }, //{ "delta", false } },
				} }, // @todo: implement delta support
				} },
		};

		return RequestSuccessResult{
			result
		};
	}

	auto ServerImplementation::operator() (notifications::DidChangeConfiguration&& msg) -> NotificationResult
	{
		// @todo: should also trigger this logic from initialize handler if starting config is given

		auto&& params = msg.params().as_object();
		if (auto const settings_iter = params.find("settings"); settings_iter != params.end() && settings_iter->value().is_object())
		{
			auto&& settings_obj = settings_iter->value().as_object();
			if (auto const build2_iter = settings_obj.find("build2"); build2_iter != settings_obj.end() && build2_iter->value().is_object())
			{
				auto&& updated_settings = build2_iter->value().as_object();
				apply_configuration_update(updated_settings);
			}
		}

		return NotificationSuccessResult{};
	}

	auto ServerImplementation::operator() (notifications::DidOpenTextDocument&& msg) -> NotificationResult
	{
		auto&& params = msg.params();

		auto const& text_doc = params.at(lsp::keys::text_document).as_object();
		auto const uri = boost::json::value_to< lsp::DocumentURI >(text_doc.at(lsp::keys::uri));

		auto const doc_iter = documents_.find(uri);
		if (doc_iter != documents_.end())
		{
			// already exists
			return std::unexpected(ResponseError{});
		}

		auto text = boost::json::value_to< lsp::DocumentContent >(text_doc.at(lsp::keys::text));
		if (auto doc = create_tracked_document(uri, std::move(text)); doc.has_value())
		{
			documents_.try_emplace(uri, std::move(*doc));
			return NotificationSuccessResult{};
		}
		else
		{
			return std::unexpected(ResponseError{});
		}
	}

	auto ServerImplementation::operator() (notifications::DidChangeTextDocument&& msg) -> NotificationResult
	{
		auto&& params = msg.params();

		auto const& text_doc_id = params.at(lsp::keys::text_document).as_object();
		auto const& changes = params.at(lsp::keys::content_changes).as_array();

		auto const uri = boost::json::value_to< std::string >(text_doc_id.at(lsp::keys::uri));

		auto const doc_iter = documents_.find(uri);
		if (doc_iter == documents_.end())
		{
			// not tracked
			return std::unexpected(ResponseError{});
		}

		auto& doc = doc_iter->second;

		for (auto&& entry : changes)
		{
			auto&& change = entry.as_object();

			if (change.contains(lsp::keys::range))
			{
				// @todo: delta handling
				continue;
			}

			update_document(doc, boost::json::value_to< std::string >(change.at(lsp::keys::text)));
		}

		return NotificationSuccessResult{};
	}

	auto ServerImplementation::operator() (notifications::DidCloseTextDocument&& msg) -> NotificationResult
	{
		auto&& params = msg.params();

		auto const& text_doc_id = params.at(lsp::keys::text_document).as_object();

		auto const uri = boost::json::value_to< std::string >(text_doc_id.at(lsp::keys::uri));

		auto const doc_iter = documents_.find(uri);
		if (doc_iter == documents_.end())
		{
			// not tracked
			return std::unexpected(ResponseError{});
		}

		documents_.erase(doc_iter);
		return NotificationSuccessResult{};
	}

	auto ServerImplementation::operator() (requests::SemanticTokensFull&& msg) -> RequestResult
	{
		auto&& params = msg.params();

		auto const& text_doc_id = boost::json::value_to< std::string >(params.at(lsp::keys::text_document).at(lsp::keys::uri));
		auto const doc_iter = documents_.find(text_doc_id);
		if (doc_iter == documents_.end())
		{
			return std::unexpected(ResponseError{});
		}

		auto const& doc = doc_iter->second;
		return invoke_on_document(doc, msg);
	}	

	auto ServerImplementation::operator() (requests::SemanticTokensRange&& msg) -> RequestResult
	{
		auto&& params = msg.params();

		auto const& text_doc_id = boost::json::value_to< std::string >(params.at(lsp::keys::text_document).at(lsp::keys::uri));
		auto const doc_iter = documents_.find(text_doc_id);
		if (doc_iter == documents_.end())
		{
			return std::unexpected(ResponseError{});
		}

		auto const& doc = doc_iter->second;
		return invoke_on_document(doc, msg);
	}
}
