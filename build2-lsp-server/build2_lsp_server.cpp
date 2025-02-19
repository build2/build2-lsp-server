
#ifdef _WIN64
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#endif

#if !defined(BUILD2_LSP_SERVER_ENABLE_IMPORT_STD)
#include <memory>
#include <iostream>
#include <thread>
#endif
#include <version>

import server_impl;

import lsp_boot;

#if defined(BUILD2_LSP_SERVER_ENABLE_IMPORT_STD)
import std;
#endif

using namespace b2lsp;

#if defined(__cpp_lib_jthread)
using Thread = std::jthread;
#else
using Thread = std::thread;
#endif

int main(int argc, char* argv[])
{
#ifdef _WIN64
	// @note: apparently the std library will auto-normalize line endings, removing \r.
	{
		auto res = _setmode(_fileno(stdin), _O_BINARY);
		if (res == -1)
		{
			std::cerr << "Error enabling stdin binary mode" << std::endl;
			return res;
		}
	}
	{
		auto res = _setmode(_fileno(stdout), _O_BINARY);
		if (res == -1)
		{
			std::cerr << "Error enabling stdout binary mode" << std::endl;
			return res;
		}
	}
#endif

	auto input_queue = lsp_boot::PendingInputQueue{};
	auto output_queue = lsp_boot::OutputQueue{};

	auto const server_impl_init = [](lsp_boot::ServerImplAPI& api) {
		return std::make_unique< ServerImplementation >(api);
		};

	auto const logger = [](lsp_boot::LogOutputCallbackView callback) {
		std::move(callback)(std::ostream_iterator< char >(std::cerr));
		std::cerr << std::endl;
		};

	auto server = lsp_boot::Server(
		input_queue,
		output_queue,
		server_impl_init,
		logger);
	auto server_thread = Thread([&] {
		server.run();
		// @TODO: although currently behaving as desired, we probably want a server exit to force the connection to close down to handle
		// corner cases/weird client behaviour.
		// We can move this server init below the connection stuff without issue, so should be easy enough.
		});

	auto connection = lsp_boot::StreamConnection(input_queue, output_queue, std::cin, std::cout, std::cerr);
	auto result = connection.listen();

	// @NOTE: Visual Studio appears not to send an LSP exit notification after the shutdown request, in the case that the IDE is closed
	// (though it does correctly send both when something causes the LSP server to be stopped/restarted).
	// So we need to ensure the server is stopped in order to be able to join all threads and exit.
	server.request_shutdown();

	return result;
}
