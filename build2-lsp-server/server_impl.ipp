
module;

#if defined(BUILD2_LSP_SERVER_ENABLE_IMPORT_STD)
import std;
#else
#include <concepts>
#include <utility>
#include <functional>
#include <memory>
#endif

export module server_impl;

import tracked_document;
import lsp_boot;
import lsp_boot.ext_mod_wrap.boost.json;

namespace b2lsp
{
	using Configuration = boost::json::object;

	export struct LoggerConfig
	{
		bool enabled = false;
	};

	export struct ServerImplementation
	{
		ServerImplementation(lsp_boot::ServerImplAPI& internal_api, std::shared_ptr< LoggerConfig > logger_config) : internal_api_{ internal_api }, logger_config_{ logger_config }
		{
		}

		~ServerImplementation();

		using Not = lsp_boot::lsp::notifications::Kinds;
		using Req = lsp_boot::lsp::requests::Kinds;

		using RequestResult = lsp_boot::Server::RequestResult;
		using RequestSuccessResult = lsp_boot::Server::RequestSuccessResult;
		using NotificationResult = lsp_boot::Server::NotificationResult;
		using NotificationSuccessResult = lsp_boot::Server::NotificationSuccessResult;
		using ResponseError = lsp_boot::Server::ResponseError;

		// @NOTE: These are invoked by the thread processing the pending input queue (requests and notifications sent from the client and queued by the connection).
		auto operator() (lsp_boot::lsp::requests::Initialize&& msg) -> RequestResult;
		auto operator() (lsp_boot::lsp::requests::SemanticTokensFull&& msg) -> RequestResult;
		auto operator() (lsp_boot::lsp::requests::SemanticTokensRange&& msg) -> RequestResult;

		auto operator() (lsp_boot::lsp::notifications::DidChangeConfiguration&& msg) -> NotificationResult;
		auto operator() (lsp_boot::lsp::notifications::DidOpenTextDocument&& msg) -> NotificationResult;
		auto operator() (lsp_boot::lsp::notifications::DidChangeTextDocument&& msg) -> NotificationResult;
		auto operator() (lsp_boot::lsp::notifications::DidCloseTextDocument&& msg) -> NotificationResult;

		auto notify_client(lsp_boot::lsp::RawMessage&& msg) const -> void
		{
			internal_api_.send_notification(std::move(msg));
		}

		auto pump() -> void;

		auto apply_configuration_update(Configuration const&) -> void;

		using DocumentMap = std::unordered_map< lsp_boot::lsp::DocumentURI, TrackedDocument >;

		lsp_boot::ServerImplAPI& internal_api_;
		Configuration configuration_;
		std::shared_ptr< LoggerConfig > logger_config_;
		DocumentMap documents_;
	};
}
