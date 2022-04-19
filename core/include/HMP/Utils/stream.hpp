#pragma once

#include <ostream>
#include <istream>
#include <concepts>
#include <type_traits>

namespace HMP::Utils
{

	template <typename TEnum> requires std::is_enum_v<TEnum>
	std::ostream& operator<<(std::ostream& _stream, TEnum _enum);

	template <typename TEnum> requires std::is_enum_v<TEnum>
	std::istream& operator>>(std::istream& _stream, TEnum& _enum);
	
}


#define HMP_UTILS_STREAM_IMPL
#include <HMP/Utils/stream.tpp>
#undef HMP_UTILS_STREAM_IMPL