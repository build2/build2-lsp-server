
module;

#if !defined(BUILD2_LSP_SERVER_ENABLE_IMPORT_STD)
#include <cstddef>
#include <cstdint>
#include <vector>
#endif

export module lang.common.semantic_tokens;

import lsp_boot;

#if defined(BUILD2_LSP_SERVER_ENABLE_IMPORT_STD)
import std;
#endif

namespace b2lsp
{
	export struct LineToken
	{
		std::size_t line_index;
		std::size_t line_char_offset;
		std::size_t length;
		lsp_boot::lsp::SemanticTokenType type;
		std::uint32_t mods = 0;
	};

	export using LineTokenList = std::vector< LineToken >;
}
