
module;

#if defined(BUILD2_LSP_SERVER_ENABLE_IMPORT_STD)
import std;
#else
#include <utility>
#include <string_view>
#endif

module server_impl;

namespace b2lsp
{
	ServerImplementation::~ServerImplementation() = default;

	auto ServerImplementation::pump() -> void
	{
	}

	auto ServerImplementation::apply_configuration_update(Configuration const& updates) -> void
	{
		auto const apply_configuration_setting = [&](std::string_view const key, boost::json::value const& value) {
			if (key == "serverLogging")
			{
				if (value.is_object())
				{
					if (auto const enabled = value.as_object().if_contains("enabled"); enabled != nullptr && enabled->is_bool())
					{
						logger_config_->enabled = enabled->as_bool();
					}
				}
			}

			configuration_[key] = std::move(value);
			};

		for (auto&& entry : updates)
		{
			apply_configuration_setting(entry.key(), entry.value());
		}
	}
}
