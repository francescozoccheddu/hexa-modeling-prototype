#include <HMP/Utils/Serialization.hpp>

namespace HMP::Utils::Serialization
{

	// Serializer

	Serializer& Serializer::operator<<(const std::string& _data)
	{
		cpputils::serialization::Serializer::operator<<(_data);
		return *this;
	}

	Serializer& Serializer::operator<<(const Vec& _data)
	{
		return *this << _data.x() << _data.y() << _data.z();
	}

	// Deserializer

	Deserializer& Deserializer::operator>>(Vec& _data)
	{
		return *this >> _data.x() >> _data.y() >> _data.z();
	}

	Deserializer& Deserializer::operator>>(std::string& _data)
	{
		cpputils::serialization::Deserializer::operator>>(_data);
		return *this;
	}

}