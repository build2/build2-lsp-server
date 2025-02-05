
module;

#if defined(BUILD2_LSP_SERVER_ENABLE_IMPORT_STD)
import std;
#else
#include <cstddef>
#include <tuple>
#include <vector>
#include <ranges>
#endif

export module utility;

export namespace b2lsp
{
	template < std::ranges::forward_range Rg >
	auto enumerate_workaround(Rg&& r) {
		std::vector< std::tuple< std::size_t, std::ranges::range_value_t< Rg > > > v;
		std::size_t i = 0;
		for (auto&& e : std::forward< Rg >(r))
		{
			v.push_back(std::tuple{ i++, e });
		}
		return v;
	};
}
