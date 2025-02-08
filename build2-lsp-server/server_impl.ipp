
module;

#if defined(BUILD2_LSP_SERVER_ENABLE_IMPORT_STD)
import std;
#else
#include <concepts>
#include <utility>
#include <functional>
#endif

export module server_impl;

import tracked_document;
import lsp_boot;

namespace b2lsp
{
	export struct ServerImplementation
	{
		template < std::regular_invocable< lsp_boot::lsp::RawMessage&& > SendNotify >
		ServerImplementation(SendNotify&& send_notify) : send_notify_{ send_notify }
		{
		}

		~ServerImplementation();

		using Not = lsp_boot::lsp::notifications::Kinds;
		using Req = lsp_boot::lsp::requests::Kinds;

		using RequestResult = lsp_boot::Server::RequestResult;
		using NotificationResult = lsp_boot::Server::NotificationResult;

		static constexpr auto result_ok = nullptr;
		static constexpr auto temp_fail = nullptr;

		// @NOTE: These are invoked by the thread processing the pending input queue (requests and notifications sent from the client and queued by the connection).
		auto operator() (lsp_boot::lsp::requests::Initialize&& msg) -> RequestResult;
		auto operator() (lsp_boot::lsp::requests::SemanticTokensFull&& msg) -> RequestResult;
		auto operator() (lsp_boot::lsp::requests::SemanticTokensRange&& msg) -> RequestResult;

		auto operator() (lsp_boot::lsp::notifications::DidOpenTextDocument&& msg) -> NotificationResult;
		auto operator() (lsp_boot::lsp::notifications::DidChangeTextDocument&& msg) -> NotificationResult;
		auto operator() (lsp_boot::lsp::notifications::DidCloseTextDocument&& msg) -> NotificationResult;

		auto notify_client(lsp_boot::lsp::RawMessage&& msg) const -> void
		{
			send_notify_(std::move(msg));
		}

		auto pump() -> void;

		using DocumentMap = std::unordered_map< lsp_boot::lsp::DocumentURI, TrackedDocument >;

		std::function< void(lsp_boot::lsp::RawMessage&&) > send_notify_;
		DocumentMap documents_;
	};
}
