#include <HMP/Gui/Widgets/VertEdit.hpp>

#include <imgui.h>
#include <stdexcept>
#include <cinolib/gl/glcanvas.h>
#include <cinolib/deg_rad.h>

namespace HMP::Gui::Widgets
{

	void VertEdit::vidsChanged()
	{
		onSelectionUpdate();
	}

	void VertEdit::apply(const Mat4& _transform, bool _notify)
	{
		for (const Id vid : m_vids)
		{
			m_mesher.moveVert(vid, _transform * m_mesher.mesh().vert(vid));
		}
		if (_notify)
		{
			onMeshUpdate();
		}
	}

	void VertEdit::applyAndCombine(const Mat4& _transform)
	{
		m_transform = _transform * m_transform;
		m_pendingAction = !isIdentity();
		apply(_transform);
	}

	void VertEdit::addOrRemove(const Id* _vids, std::size_t _count, bool _add)
	{
		bool changed{ false };
		for (const Id* vidp{ _vids }; vidp < _vids + _count; vidp++)
		{
			const Id vid{ *vidp };
			const auto it{ m_vids.find(vid) };
			if ((it == m_vids.end()) == _add)
			{
				if (!changed)
				{
					changed = true;
					applyAction();
				}
				const Vec vert{ m_mesher.mesh().vert(vid) };
				m_centroid *= static_cast<Real>(m_vids.size());
				if (_add)
				{
					m_vids.insert(it, vid);
					m_centroid += vert;
				}
				else
				{
					m_vids.erase(it);
					m_centroid -= vert;
				}
				if (!empty())
				{
					m_centroid /= static_cast<Real>(m_vids.size());
				}
				else
				{
					m_centroid = {};
				}
			}
		}
		if (changed)
		{
			m_pendingAction &= !empty();
			m_mesher.updateMesh();
			vidsChanged();
		}
	}

	bool VertEdit::isIdentity() const
	{
		static constexpr Real eps{ 1e-9 };
		for (unsigned int r{}; r < 4; r++)
		{
			for (unsigned int c{}; c < 4; c++)
			{
				const Real val{ m_transform(r,c) };
				const Real idVal{ static_cast<Real>(r == c ? 1.0 : 0.0) };
				if (val < idVal - eps || val > idVal + eps)
				{
					return false;
				}
			}
		}
		return true;
	}

	VertEdit::VertEdit(Meshing::Mesher& _mesher) : cinolib::SideBarItem{ "Vertex editor" }, m_mesher{ _mesher }, m_centroid{}, m_vids{}, m_pendingAction{ false }
	{ }

	void VertEdit::add(Id _vid)
	{
		addOrRemove(&_vid, 1, true);
	}

	void VertEdit::remove(Id _vid)
	{
		addOrRemove(&_vid, 1, false);
	}

	void VertEdit::add(const std::vector<Id>& _vids)
	{
		if (!_vids.empty())
		{
			addOrRemove(&_vids[0], _vids.size(), true);
		}
	}

	void VertEdit::remove(const std::vector<Id>& _vids)
	{
		if (!_vids.empty())
		{
			addOrRemove(&_vids[0], _vids.size(), false);
		}
	}

	bool VertEdit::has(Id _vid) const
	{
		return m_vids.contains(_vid);
	}

	const std::unordered_set<Id>& VertEdit::vids() const
	{
		return m_vids;
	}

	void VertEdit::clear()
	{
		applyAction();
		m_vids.clear();
		vidsChanged();
	}

	bool VertEdit::empty() const
	{
		return m_vids.empty();
	}

	const Vec& VertEdit::centroid() const
	{
		if (empty())
		{
			throw std::logic_error{ "empty" };
		}
		return m_centroid;
	}

	bool VertEdit::pendingAction() const
	{
		return m_pendingAction;
	}

	void VertEdit::applyAction()
	{
		if (m_pendingAction)
		{
			apply(m_transform.inverse(), false);
			m_pendingAction = false;
			onApplyAction(m_vids, m_transform);
		}
		m_transform = Mat4::DIAG(Vec4{ 1 });
	}

	void VertEdit::translate(const Vec& _offset)
	{
		if (!m_vids.empty())
		{
			m_centroid += _offset;
			applyAndCombine(Mat4::TRANS(_offset));
		}
	}

	void VertEdit::rotate(const Vec& _normAxis, Real _angleDeg)
	{
		if (m_vids.size() > 1)
		{
			const Mat4 toCentroid{ Mat4::TRANS(m_centroid) };
			const Mat3 rotate{ Mat3::ROT_3D(_normAxis, cinolib::to_rad(_angleDeg)) };
			const Mat4 rotateHom{
				rotate(0,0),	rotate(0,1),	rotate(0,2),	0,
				rotate(1,0),	rotate(1,1),	rotate(1,2),	0,
				rotate(2,0),	rotate(2,1),	rotate(2,2),	0,
				0,				0,				0,				1
			};
			const Mat4 fromCentroid{ Mat4::TRANS(-m_centroid) };
			applyAndCombine(toCentroid * rotateHom * fromCentroid);
		}
	}

	void VertEdit::scale(const Vec& _amount)
	{
		if (m_vids.size() > 1)
		{
			const Mat4 toCentroid{ Mat4::TRANS(m_centroid) };
			const Mat4 scaleHom(Mat4::DIAG(_amount.add_coord(1.0)));
			const Mat4 fromCentroid{ Mat4::TRANS(-m_centroid) };
			applyAndCombine(toCentroid * scaleHom * fromCentroid);
		}
	}

	void VertEdit::draw()
	{
		ImGui::TextDisabled("%d vertices selected", static_cast<int>(m_vids.size()));
	}

}