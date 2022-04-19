#ifndef HMP_UTILS_STREAM_IMPL
#error __FILE__ should not be directly included
#endif

#include <HMP/Utils/stream.hpp>

namespace HMP::Utils
{

	template<typename TEnum> requires std::is_enum_v<TEnum>
	std::ostream& operator<<(std::ostream& _stream, TEnum _enum)
	{
		return _stream << static_cast<std::underlying_type_t<TEnum>>(_enum);
	}

	template<typename TEnum> requires std::is_enum_v<TEnum>
	std::istream& operator>>(std::istream& _stream, TEnum& _enum)
	{
		std::underlying_type_t<TEnum> underlying;
		_stream >> underlying;
		_enum = static_cast<TEnum>(underlying);
		return _stream;
	}

}