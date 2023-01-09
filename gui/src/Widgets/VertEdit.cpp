#include <HMP/Gui/Widgets/VertEdit.hpp>

#include <imgui.h>
#include <cassert>
#include <HMP/Gui/Utils/Controls.hpp>
#include <HMP/Gui/Utils/Drawing.hpp>
#include <HMP/Gui/themer.hpp>
#include <HMP/Gui/App.hpp>
#include <HMP/Meshing/Utils.hpp>

namespace HMP::Gui::Widgets
{

	bool VertEdit::addOrRemove(const Id* _vids, I _count, bool _add, bool _update)
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
		if (changed && _update)
		{
			onVidsChanged();
			updateCentroid();
		}
		return changed;
	}

	VertEdit::VertEdit(Meshing::Mesher& _mesher):
		SidebarWidget{ "Vertex editor" }, m_mesher{ _mesher },
		m_verts{}, m_pendingAction{ false },
		m_unappliedTransform{}, m_appliedTransform{}, m_centroid{}
	{ }

	bool VertEdit::add(Id _vid, bool _update)
	{
		return addOrRemove(&_vid, 1, true, _update);
	}

	bool VertEdit::remove(Id _vid, bool _update)
	{
		return addOrRemove(&_vid, 1, false, _update);
	}

	bool VertEdit::add(const std::vector<Id>& _vids, bool _update)
	{
		if (!_vids.empty())
		{
			return addOrRemove(&_vids[0], _vids.size(), true, _update);
		}
		return false;
	}

	bool VertEdit::remove(const std::vector<Id>& _vids, bool _update)
	{
		if (!_vids.empty())
		{
			return addOrRemove(&_vids[0], _vids.size(), false, _update);
		}
		return false;
	}

	bool VertEdit::has(Id _vid) const
	{
		return m_verts.contains(_vid);
	}

	VertEdit::Vids VertEdit::vids() const
	{
		return cpputils::range::ofc(m_verts).map(&vertsToVidsConvert);
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
		assert(!empty());
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
			m_centroid /= static_cast<Real>(m_verts.size());
			m_unappliedTransform.origin /= static_cast<Real>(m_verts.size());
		}
		onCentroidChanged();
	}

	void VertEdit::drawSidebar()
	{
		ImGui::TextColored(m_verts.empty() ? themer->sbWarn : themer->sbOk, "%d vertices selected", static_cast<int>(m_verts.size()));
		if (empty())
		{
			return;
		}
		ImGui::Spacing();
		ImGui::BeginTable("Transform", 2, ImGuiTableFlags_RowBg);
		ImGui::TableSetupColumn("drag", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("button", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableNextColumn();
		ImGui::Text("Transform");
		ImGui::TableNextColumn();
		if (ImGui::Button("Reset##all"))
		{
			cancel();
		}
		// translation
		ImGui::TableNextColumn();
		if (Utils::Controls::dragTranslationVec("Translation", m_unappliedTransform.translation, m_mesher.mesh().scene_radius()))
		{
			applyTransform();
		}
		ImGui::TableNextColumn();
		if (ImGui::Button("Reset##translation"))
		{
			m_unappliedTransform.translation = Vec{};
			applyTransform();
		}
		// scale
		ImGui::TableNextColumn();
		if (Utils::Controls::dragScaleVec("Scale", m_unappliedTransform.scale))
		{
			applyTransform();
		}
		ImGui::TableNextColumn();
		if (ImGui::Button("Reset##scale"))
		{
			m_unappliedTransform.scale = { 1.0 };
			applyTransform();
		}
		// rotation
		ImGui::TableNextColumn();
		if (Utils::Controls::dragRotation("Rotation", m_unappliedTransform.rotation))
		{
			applyTransform();
		}
		ImGui::TableNextColumn();
		if (ImGui::Button("Reset##rotation"))
		{
			m_unappliedTransform.rotation = Vec{};
			applyTransform();
		}
		ImGui::EndTable();
	}

	void VertEdit::draw(const cinolib::GLcanvas& _canvas)
	{
		const float
			radius{ this->radius * themer->ovScale },
			lineThickness{ this->lineThickness * themer->ovScale };
		ImDrawList& drawList{ *ImGui::GetWindowDrawList() };
		for (const Id vid : vids())
		{
			const Vec vert{ m_mesher.mesh().vert(vid) };
			const auto pos{ Utils::Drawing::project(_canvas, vert) };
			Utils::Drawing::circle(drawList, pos, radius, themer->ovHi, lineThickness);
		}
		if (!empty())
		{
			Utils::Drawing::cross(drawList, Utils::Drawing::project(_canvas, m_centroid), radius, themer->ovHi, lineThickness);
			const char* verticesLit{ m_verts.size() == 1 ? "vertex" : "vertices" };
			const int vertexCount{ static_cast<int>(m_verts.size()) };
			ImGui::TextColored(Utils::Drawing::toImVec4(themer->ovMut), "%d %s selected", vertexCount, verticesLit);
		}
	}

	bool VertEdit::keyPressed(const cinolib::KeyBinding& _key)
	{
		if (_key == c_kbSelectVertex)
		{
			onSelect(ESelectionSource::Vertex, ESelectionMode::Set);
		}
		else if (_key == c_kbSelectEdge)
		{
			onSelect(ESelectionSource::Edge, ESelectionMode::Set);
		}
		else if (_key == c_kbSelectUpEdge)
		{
			onSelect(ESelectionSource::UpEdge, ESelectionMode::Set);
		}
		else if (_key == c_kbSelectFace)
		{
			onSelect(ESelectionSource::Face, ESelectionMode::Set);
		}
		else if (_key == c_kbSelectUpFace)
		{
			onSelect(ESelectionSource::UpFace, ESelectionMode::Set);
		}
		else if (_key == c_kbSelectPoly)
		{
			onSelect(ESelectionSource::Poly, ESelectionMode::Set);
		}
		else if (_key == (c_kbSelectVertex | c_kmodSelectAdd))
		{
			onSelect(ESelectionSource::Vertex, ESelectionMode::Add);
		}
		else if (_key == (c_kbSelectEdge | c_kmodSelectAdd))
		{
			onSelect(ESelectionSource::Edge, ESelectionMode::Add);
		}
		else if (_key == (c_kbSelectUpEdge | c_kmodSelectAdd))
		{
			onSelect(ESelectionSource::UpEdge, ESelectionMode::Add);
		}
		else if (_key == (c_kbSelectFace | c_kmodSelectAdd))
		{
			onSelect(ESelectionSource::Face, ESelectionMode::Add);
		}
		else if (_key == (c_kbSelectUpFace | c_kmodSelectAdd))
		{
			onSelect(ESelectionSource::UpFace, ESelectionMode::Add);
		}
		else if (_key == (c_kbSelectPoly | c_kmodSelectAdd))
		{
			onSelect(ESelectionSource::Poly, ESelectionMode::Add);
		}
		else if (_key == (c_kbSelectVertex | c_kmodSelectRemove))
		{
			onSelect(ESelectionSource::Vertex, ESelectionMode::Remove);
		}
		else if (_key == (c_kbSelectEdge | c_kmodSelectRemove))
		{
			onSelect(ESelectionSource::Edge, ESelectionMode::Remove);
		}
		else if (_key == (c_kbSelectUpEdge | c_kmodSelectRemove))
		{
			onSelect(ESelectionSource::UpEdge, ESelectionMode::Remove);
		}
		else if (_key == (c_kbSelectFace | c_kmodSelectRemove))
		{
			onSelect(ESelectionSource::Face, ESelectionMode::Remove);
		}
		else if (_key == (c_kbSelectUpFace | c_kmodSelectRemove))
		{
			onSelect(ESelectionSource::UpFace, ESelectionMode::Remove);
		}
		else if (_key == (c_kbSelectPoly | c_kmodSelectRemove))
		{
			onSelect(ESelectionSource::Poly, ESelectionMode::Remove);
		}
		else if (_key == c_kbDeselectAll)
		{
			onSelectAll(false);
		}
		else if (_key == c_kbSelectAll)
		{
			onSelectAll(true);
		}
		else
		{
			return false;
		}
		return true;
	}

	void VertEdit::onSelect(ESelectionSource _source, ESelectionMode _mode)
	{
		const App::Mouse& mouse{ app().mouse() };
		if (mouse.element)
		{
			std::vector<Id> vids{};
			switch (_source)
			{
				case ESelectionSource::Vertex:
					vids = { mouse.vid };
					break;
				case ESelectionSource::Edge:
					vids = app().mesher.mesh().edge_vert_ids(mouse.eid);
					break;
				case ESelectionSource::Face:
					vids = app().mesher.mesh().face_verts_id(mouse.fid);
					break;
				case ESelectionSource::UpFace:
					vids = app().mesher.mesh().face_verts_id(Meshing::Utils::adjFidInPidByFidAndEid(app().mesher.mesh(), mouse.pid, mouse.fid, mouse.eid));
					break;
				case ESelectionSource::UpEdge:
					vids = { mouse.vid, app().mesher.mesh().poly_vert_opposite_to(mouse.pid, mouse.fid, mouse.vid) };
					break;
				case ESelectionSource::Poly:
					vids = app().mesher.mesh().poly_verts_id(mouse.pid);
					break;
			}
			if (_mode == ESelectionMode::Set)
			{
				clear();
			}
			if (_mode == ESelectionMode::Remove)
			{
				remove(vids);
			}
			else
			{
				add(vids);
			}
		}
	}

	void VertEdit::onSelectAll(bool _selected)
	{
		if (_selected)
		{
			std::vector<Id> vids(toI(app().mesher.mesh().num_verts()));
			for (I i{}; i < vids.size(); i++)
			{
				vids[i] = toId(i);
			}
			add(vids);
		}
		else
		{
			clear();
		}
	}

	void VertEdit::printUsage() const
	{
		cinolib::print_binding(c_kbSelectVertex.name(), "select vertex");
		cinolib::print_binding(c_kbSelectEdge.name(), "select edge vertices");
		cinolib::print_binding(c_kbSelectUpEdge.name(), "select adjacent edge vertices");
		cinolib::print_binding(c_kbSelectFace.name(), "select face vertices");
		cinolib::print_binding(c_kbSelectUpFace.name(), "select adjacent face vertices");
		cinolib::print_binding(c_kbSelectPoly.name(), "select poly vertices");
		cinolib::print_binding(cinolib::KeyBinding::mod_names(c_kmodSelectAdd), "remove from selection (hold down)");
		cinolib::print_binding(cinolib::KeyBinding::mod_names(c_kmodSelectRemove), "add to selection (hold down)");
		cinolib::print_binding(c_kbSelectAll.name(), "select all vertices");
		cinolib::print_binding(c_kbDeselectAll.name(), "deselect all vertices");
	}

}