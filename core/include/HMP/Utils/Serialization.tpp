#ifndef HMP_UTILS_SERIALIZATION_IMPL
#error __FILE__ should not be directly included
#endif

#include <HMP/Utils/Serialization.hpp>

namespace HMP::Utils::Serialization
{

	// Serializer

	template <typename TArithmetic> requires std::is_arithmetic_v<TArithmetic>
	Serializer& Serializer::operator<<(TArithmetic _data)
	{
		cpputils::serialization::Serializer::operator<<(_data);
		return *this;
	}

	template <typename TEnum> requires std::is_enum_v<TEnum>
	Serializer& Serializer::operator<<(TEnum _data)
	{

		cpputils::serialization::Serializer::operator<<(_data);
		return *this;
	}


	// Deserializer


	template <typename TArithmetic> requires std::is_arithmetic_v<TArithmetic> && (!std::is_const_v<TArithmetic>)
		Deserializer& Deserializer::operator>>(TArithmetic& _data)
	{
		cpputils::serialization::Deserializer::operator>>(_data);
		return *this;
	}

	template <typename TEnum> requires std::is_enum_v<TEnum> && (!std::is_const_v<TEnum>)
		Deserializer& Deserializer::operator>>(TEnum& _data)
	{
		cpputils::serialization::Deserializer::operator>>(_data);
		return *this;
	}

	template <typename TType> requires std::is_arithmetic_v<TType> || std::is_enum_v<TType> || std::is_same_v<std::remove_cv_t<TType>, std::string> || std::is_same_v<std::remove_cv_t<TType>, Vec>
	TType Deserializer::get()
	{
		std::remove_cv_t<TType> data;
		*this >> data;
		return data;
	}

}