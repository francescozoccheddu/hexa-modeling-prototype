#pragma once

#include <HMP/Meshing/types.hpp>
#include <cpputils/serialization/Serializer.hpp>
#include <cpputils/serialization/Deserializer.hpp>

namespace HMP::Utils::Serialization
{

	class Serializer final : public cpputils::serialization::Serializer
	{

	public:

		using cpputils::serialization::Serializer::Serializer;

		template <typename TArithmetic> requires std::is_arithmetic_v<TArithmetic>
		Serializer& operator<<(TArithmetic _data);

		template <typename TEnum> requires std::is_enum_v<TEnum>
		Serializer& operator<<(TEnum _data);

		Serializer& operator<<(const std::string& _data);

		Serializer& operator<<(const Vec& _data);

	};

	class Deserializer final : public cpputils::serialization::Deserializer
	{

	public:

		using cpputils::serialization::Deserializer::Deserializer;

		template <typename TArithmetic> requires std::is_arithmetic_v<TArithmetic> && (!std::is_const_v<TArithmetic>)
			Deserializer& operator>>(TArithmetic& _data);

		template <typename TEnum> requires std::is_enum_v<TEnum> && (!std::is_const_v<TEnum>)
			Deserializer& operator>>(TEnum& _data);

		Deserializer& operator>>(std::string& _data);

		Deserializer& operator>>(Vec& _data);

		template <typename TType> requires std::is_arithmetic_v<TType> || std::is_enum_v<TType> || std::is_same_v<std::remove_cv_t<TType>, std::string> || std::is_same_v<std::remove_cv_t<TType>, Vec>
		TType get();

	};

}

#define HMP_UTILS_SERIALIZATION_IMPL
#include <HMP/Utils/Serialization.tpp>
#undef HMP_UTILS_SERIALIZATION_IMPL