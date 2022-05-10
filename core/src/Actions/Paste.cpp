#include <HMP/Actions/Paste.hpp>

#include <HMP/Meshing/Utils.hpp>
#include <algorithm>

namespace HMP::Actions
{

	Paste::~Paste()
	{}

	void Paste::apply()
	{}

	void Paste::unapply()
	{}

	Paste::Paste(Dag::Element& _target, Id _targetForwardFaceOffset, Id _targetUpFaceOffset, Dag::Extrude& _source, Id _sourceUpFaceOffset)
		: m_operation{ *new Dag::Extrude{} }
	{}

}