
#include <fcntl.h>
#include <io.h>
#include <stdio.h>

import lsp_boot;
import server_impl;

#if defined(BUILD2_LSP_SERVER_ENABLE_IMPORT_STD)
import std;
#else
#include <memory>
#include <iostream>
#include <thread>
#endif

using namespace b2lsp;

int main(int argc, char* argv[])
{
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

	auto input_queue = lsp_boot::PendingInputQueue{};
	auto output_queue = lsp_boot::OutputQueue{};

	auto server_impl_init = [](auto&& send_notify) {
		return std::make_unique< ServerImplementation >(send_notify);
		};

	auto server = lsp_boot::Server(input_queue, output_queue, server_impl_init);
	auto server_thread = std::jthread([&] {
		server.run();
		});

	auto connection = lsp_boot::StreamConnection(input_queue, output_queue, std::cin, std::cout, std::cerr);
	return connection.listen();
}
