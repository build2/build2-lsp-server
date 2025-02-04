
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
				{ "semanticTokensProvider", { { "legend", semantic_tokens_legend }, { "full", true } } }, // @todo: implement delta support
				} },
		};

		return result;
	}

	auto ServerImplementation::operator() (notifications::DidOpenTextDocument&& msg) -> NotificationResult
	{
		auto&& params = msg.params();

		auto const& text_doc = params.at(lsp::keys::text_document).as_object();
		auto const uri = boost::json::value_to< lsp::DocumentURI >(text_doc.at(lsp::keys::uri));

		auto text = boost::json::value_to< lsp::DocumentContent >(text_doc.at(lsp::keys::text));


		return result_ok;
	}

	auto ServerImplementation::operator() (notifications::DidChangeTextDocument&& msg) -> NotificationResult
	{
		auto&& params = msg.params();

		auto const& text_doc_id = params.at(lsp::keys::text_document).as_object();
		auto const& changes = params.at(lsp::keys::content_changes).as_array();

		auto const uri = boost::json::value_to< std::string >(text_doc_id.at(lsp::keys::uri));
		
		return result_ok;
	}

	auto ServerImplementation::operator() (notifications::DidCloseTextDocument&& msg) -> NotificationResult
	{
		auto&& params = msg.params();

		auto const& text_doc_id = params.at(lsp::keys::text_document).as_object();

		auto const uri = boost::json::value_to< std::string >(text_doc_id.at(lsp::keys::uri));

		return result_ok;
	}

	auto ServerImplementation::operator() (requests::SemanticTokens&& msg) -> RequestResult
	{
		auto&& params = msg.params();

		auto const& text_doc_id = boost::json::value_to< std::string >(params.at(lsp::keys::text_document).at(lsp::keys::uri));

		auto result = boost::json::object{
			// @todo: resultId for deltas: https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#semanticTokensLegend
			{ "data", boost::json::array() },
		};
		return result;
	}	
}
