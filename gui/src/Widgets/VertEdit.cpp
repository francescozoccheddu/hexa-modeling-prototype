#include <HMP/Gui/Widgets/VertEdit.hpp>

#include <imgui.h>
#include <stdexcept>
#include <cinolib/gl/glcanvas.h>
#include <cinolib/deg_rad.h>
#include <limits>
#include <utility>

namespace HMP::Gui::Widgets
{

	bool VertEdit::apply(const Mat4& _transform, bool _updateMesh)
	{
		std::vector<std::pair<Id, Vec>> backup{};
		backup.reserve(m_vids.size());
		bool succeeded{ true };
		for (const Id vid : m_vids)
		{
			const Vec oldPos{ m_mesher.mesh().vert(vid) };
			backup.push_back({ vid,oldPos });
			const Vec newPos{ _transform * oldPos };
			const Id vidAtNewPos{ m_mesher.getVert(newPos) };
			if (vidAtNewPos != noId && vidAtNewPos != vid)
			{
				succeeded = false;
				break;
			}
			m_mesher.moveVert(vid, newPos);
		}
		if (succeeded)
		{
			if (_updateMesh)
			{
				m_mesher.updateMesh();
				onMeshUpdated();
			}
		}
		else
		{
			for (std::size_t iPlusOne{ backup.size() }; iPlusOne > 0; iPlusOne--)
			{
				const auto& [vid, oldPos] {backup[iPlusOne - 1]};
				m_mesher.moveVert(vid, oldPos);
			}
		}
		return succeeded;
	}

	bool VertEdit::applyAndCombine(const Mat4& _transform)
	{
		if (apply(_transform))
		{
			m_transform = _transform * m_transform;
			const bool hadPendingAction{ m_pendingAction };
			m_pendingAction = !isIdentity();
			if (m_pendingAction != hadPendingAction)
			{
				onPendingActionChanged();
			}
			return true;
		}
		return false;
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
			onVidsChanged();
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

	void VertEdit::cancel(bool _updateMesh)
	{
		if (!apply(m_transform.inverse(), _updateMesh))
		{
			throw std::logic_error{ "cannot revert" };
		}
		m_translationSlider = {};
		m_rotationSlider = {};
		m_scaleSlider = { 1.0f };
		m_transform = Mat4::DIAG(Vec4{ 1 });
		update();
		if (m_pendingAction)
		{
			m_pendingAction = false;
			onPendingActionChanged();
		}
	}

	VertEdit::VertEdit(Meshing::Mesher& _mesher) :
		cinolib::SideBarItem{ "Vertex editor" }, m_mesher{ _mesher },
		m_centroid{}, m_vids{}, m_pendingAction{ false },
		m_transform{ Mat4::DIAG({1}) }, m_translationSlider{}, m_scaleSlider{}, m_rotationSlider{}
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
		onVidsChanged();
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

	void VertEdit::cancel()
	{
		cancel(true);
	}

	void VertEdit::applyAction()
	{
		if (m_pendingAction)
		{
			const Mat4 transform{ m_transform };
			cancel(false);
			onApplyAction(m_vids, transform);
		}
	}

	bool VertEdit::translate(const Vec& _offset)
	{
		if (!m_vids.empty())
		{
			if (applyAndCombine(Mat4::TRANS(_offset)))
			{
				m_centroid += _offset;
				onCentroidChanged();
				return true;
			}
		}
		return false;
	}

	bool VertEdit::rotate(const Vec& _normAxis, Real _angleDeg)
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
			return applyAndCombine(toCentroid * rotateHom * fromCentroid);
		}
		return false;
	}

	bool VertEdit::scale(const Vec& _amount)
	{
		if (m_vids.size() > 1)
		{
			const Mat4 toCentroid{ Mat4::TRANS(m_centroid) };
			const Mat4 scaleHom(Mat4::DIAG(_amount.add_coord(1.0)));
			const Mat4 fromCentroid{ Mat4::TRANS(-m_centroid) };
			return applyAndCombine(toCentroid * scaleHom * fromCentroid);
		}
		return false;
	}

	void VertEdit::update()
	{
		m_centroid = {};
		if (!empty())
		{
			for (const Id vid : m_vids)
			{
				m_centroid += m_mesher.mesh().vert(vid);
			}
			m_centroid /= m_vids.size();
		}
		onCentroidChanged();
	}

	void VertEdit::draw()
	{
		ImGui::TextDisabled("%d vertices selected", static_cast<int>(m_vids.size()));
		if (empty())
		{
			return;
		}
		const float size{ m_mesher.mesh().scene_radius() };
		const cinolib::vec3<float> lastTranslation{ m_translationSlider };
		if (ImGui::DragFloat3("Translation", m_translationSlider.ptr(), size / 1000.0f, 0.0f, 0.0f, "%.3f,"))
		{
			if (!translate((m_translationSlider - lastTranslation).cast<Real>()))
			{
				m_translationSlider = lastTranslation;
			}
		}
		ImGui::SameLine();
		if (ImGui::SmallButton("Reset##translation"))
		{
			if (translate(-m_translationSlider.cast<Real>()))
			{
				m_translationSlider = {};
			}
		}
		if (ImGui::Button("Reset##all"))
		{
			cancel();
		}
	}

}