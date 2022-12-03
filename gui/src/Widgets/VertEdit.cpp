#include <HMP/Gui/Widgets/VertEdit.hpp>

#include <imgui.h>
#include <stdexcept>
#include <HMP/Gui/Utils/Controls.hpp>

namespace HMP::Gui::Widgets
{

	void VertEdit::addOrRemove(const Id* _vids, I _count, bool _add)
	{
		bool changed{ false };
		for (const Id* vidp{ _vids }; vidp < _vids + _count; vidp++)
		{
			const Id vid{ *vidp };
			const auto it{ m_verts.find(vid) };
			if ((it == m_verts.end()) == _add)
			{
				if (!changed)
				{
					changed = true;
					applyAction();
				}
				if (_add)
				{
					const Vec pos{ m_mesher.mesh().vert(vid) };
					m_verts.insert(it, { vid, pos });
				}
				else
				{
					m_verts.erase(it);
				}
			}
		}
		if (changed)
		{
			onVidsChanged();
			updateCentroid();
		}
	}

	VertEdit::VertEdit(Meshing::Mesher& _mesher) :
		cinolib::SideBarItem{ "Vertex editor" }, m_mesher{ _mesher },
		m_verts{}, m_pendingAction{ false },
		m_unappliedTransform{}, m_appliedTransform{}, m_centroid{}
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
		return m_verts.contains(_vid);
	}

	VertEdit::Vids VertEdit::vids() const
	{
		return Vids{ m_verts };
	}

	void VertEdit::clear()
	{
		applyAction();
		m_verts.clear();
		onVidsChanged();
	}

	bool VertEdit::empty() const
	{
		return m_verts.empty();
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
			const Mat4 transform{ m_appliedTransform.matrix() };
			std::vector<Id> vids{};
			vids.reserve(m_verts.size());
			for (const auto& [vid, pos] : m_verts)
			{
				vids.push_back(vid);
			}
			cancel();
			onApplyAction(vids, transform);
			for (auto& [vid, pos] : m_verts)
			{
				pos = m_mesher.mesh().vert(vid);
			}
			updateCentroid();
		}
	}

	Utils::Transform& VertEdit::transform()
	{
		return m_unappliedTransform;
	}

	const Utils::Transform& VertEdit::transform() const
	{
		return m_unappliedTransform;
	}

	void VertEdit::applyTransform()
	{
		const Mat4 transform{ m_unappliedTransform.matrix() };
		std::unordered_set<Id> vids{};
		vids.reserve(m_verts.size());
		for (const auto& [vid, pos] : m_verts)
		{
			m_mesher.moveVert(vid, transform * pos);
			vids.insert(vid);
		}
		m_appliedTransform = m_unappliedTransform;
		m_mesher.updateMeshTemp(vids);
		onMeshUpdated();
		updateCentroid();
		const bool hadPendingAction{ m_pendingAction };
		m_pendingAction = !m_appliedTransform.isIdentity();
		if (m_pendingAction != hadPendingAction)
		{
			onPendingActionChanged();
		}
	}

	void VertEdit::cancel()
	{
		m_unappliedTransform = {};
		applyTransform();
	}

	void VertEdit::updateCentroid()
	{
		m_centroid = m_unappliedTransform.origin = Vec{};
		if (!empty())
		{
			for (const auto& [vid, pos] : m_verts)
			{
				m_unappliedTransform.origin += pos;
				m_centroid += m_mesher.mesh().vert(vid);
			}
			m_centroid /= m_verts.size();
			m_unappliedTransform.origin /= m_verts.size();
		}
		onCentroidChanged();
	}

	void VertEdit::draw()
	{
		ImGui::TextDisabled("%d vertices selected", static_cast<int>(m_verts.size()));
		if (empty())
		{
			return;
		}
		// translation
		if (Utils::Controls::dragTranslationVec("Translation", m_unappliedTransform.translation, m_mesher.mesh().scene_radius()))
		{
			applyTransform();
		}
		ImGui::SameLine();
		if (ImGui::SmallButton("Reset##translation"))
		{
			m_unappliedTransform.translation = Vec{};
			applyTransform();
		}
		// scale
		if (Utils::Controls::dragScaleVec("Scale", m_unappliedTransform.scale))
		{
			applyTransform();
		}
		ImGui::SameLine();
		if (ImGui::SmallButton("Reset##scale"))
		{
			m_unappliedTransform.scale = { 1.0 };
			applyTransform();
		}
		// rotation
		if (Utils::Controls::dragRotation("Rotation", m_unappliedTransform.rotation))
		{
			applyTransform();
		}
		ImGui::SameLine();
		if (ImGui::SmallButton("Reset##rotation"))
		{
			m_unappliedTransform.rotation = Vec{};
			applyTransform();
		}
		if (ImGui::Button("Reset##all"))
		{
			cancel();
		}
	}

}