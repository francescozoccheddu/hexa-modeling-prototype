#include <HMP/Gui/Widgets/Projection.hpp>

#include <imgui.h>
#include <utility>
#include <stdexcept>
#include <HMP/Actions/Project.hpp>
#include <cinolib/export_surface.h>
#include <limits>
#include <algorithm>
#include <set>

namespace HMP::Gui::Widgets
{

	Projection::Projection(Widgets::Target& _targetWidget, HMP::Commander& _commander, Meshing::Mesher& _mesher, VertEdit& _vertEditWidget) :
		cinolib::SideBarItem{ "Projection" }, m_targetWidget{ _targetWidget }, m_commander{ _commander }, m_mesher{ _mesher }, m_vertEditWidget{ _vertEditWidget },
		onProjectRequest{}, m_options{}
	{
		m_targetWidget.onMeshChanged += [this]() {
			updateTargetMeshEdges(0, m_creases.size(), false);
			for (EdgeChainPair& pair : m_creases)
			{
				pair.target.clear();
			}
		};
		m_mesher.onElementAdd += [this](const Dag::Element& _element) {
			I i{};
			for (EdgeChainPair& pair : m_creases)
			{
				for (const Id& eid : pair.source)
				{
					if (!m_mesher.mesh().edge_is_on_srf(eid))
					{
						updateSourceMeshEdges(i, i + 1, false);
						pair.source.clear();
						break;
					}
				}
				i++;
			}
		};
		m_mesher.onElementRemove += [this](const Dag::Element& _element, const Meshing::Mesher::RemovedIds& _removedIds) {
			Id lastEid{ m_mesher.mesh().num_edges() };
			for (const Id removedEid : _removedIds.eids)
			{
				lastEid--;
				I i{};
				for (EdgeChainPair& pair : m_creases)
				{
					for (Id& eid : pair.source)
					{
						if (eid == lastEid)
						{
							eid = removedEid;
						}
						else if (eid == removedEid)
						{
							updateSourceMeshEdges(i, i + 1, false);
							pair.source.clear();
							break;
						}
					}
					i++;
				}
			}
		};
	}

	const Algorithms::Projection::Options& Projection::options() const
	{
		return m_options;
	}

	void Projection::requestProjection()
	{
		if (!m_targetWidget.hasMesh())
		{
			throw std::logic_error{ "no target mesh" };
		}
		onProjectRequest(m_options);
	}

	bool Projection::canReproject() const
	{
		if (!m_commander.canUndo())
		{
			return false;
		}
		return dynamic_cast<const Actions::Project*>(&m_commander.applied().first());
	}

	void Projection::requestReprojection()
	{
		if (!canReproject())
		{
			throw std::logic_error{ "no projection to undo" };
		}
		m_commander.undo();
		requestProjection();
	}

	void Projection::matchCreases(I _first, I _lastEx, bool _fromSource)
	{
		// TODO Fix findCreases first
	}

	void Projection::findCreases(bool _inSource)
	{
		std::vector<std::vector<Id>> network{};
		if (_inSource)
		{
			updateSourceMeshEdges(0, m_creases.size(), false);
			for (EdgeChainPair& pair : m_creases)
			{
				pair.source.clear();
			}
			cinolib::Polygonmesh<> surface{};
			cinolib::export_surface(m_mesher.mesh(), surface);
			cinolib::feature_network(surface, network, m_featureFinderOptions);
		}
		else
		{
			updateTargetMeshEdges(0, m_creases.size(), false);
			for (EdgeChainPair& pair : m_creases)
			{
				pair.target.clear();
			}
			cinolib::feature_network(m_targetWidget.meshForDisplay(), network, m_featureFinderOptions);
		}
		std::cout << network.size() << std::endl;
		// FIXME Why network is empty?
		// TODO Clear
	}

	void Projection::updateMeshEdges(I _first, I _lastEx, bool _show)
	{
		updateSourceMeshEdges(_first, _lastEx, _show);
		updateTargetMeshEdges(_first, _lastEx, _show);
	}

	void Projection::updateSourceMeshEdges(I _first, I _lastEx, bool _show)
	{
		for (I i{ _first }; i < _lastEx; i++)
		{
			const cinolib::Color& color{ _show
				? cinolib::Color::hsv2rgb(static_cast<float>(i) / static_cast<float>(m_creases.size()), 1.0f, 1.0f)
				: m_mesher.edgeColor()
			};
			for (const Id eid : m_creases[i].source)
			{
				m_mesher.paintEdge(eid, color);
			}
		}
	}

	void Projection::updateTargetMeshEdges(I _first, I _lastEx, bool _show)
	{
		for (I i{ _first }; i < _lastEx; i++)
		{
			const cinolib::Color& color{ _show
				? cinolib::Color::hsv2rgb(static_cast<float>(i) / static_cast<float>(m_creases.size()), 1.0f, 1.0f)
				: m_targetWidget.edgeColor()
			};
			for (const Id eid : m_creases[i].target)
			{
				m_targetWidget.paintEdge(eid, color);
			}
		}
	}

	void Projection::setSourceCreaseFromSelection(I _crease)
	{
		updateSourceMeshEdges(_crease, _crease + 1, false);
		EdgeChain& crease{ m_creases[_crease].source };
		crease.clear();
		// TODO 
		updateSourceMeshEdges(_crease, _crease + 1, true);
	}

	void Projection::setTargetCreaseEdgeAtPoint(const Vec& _point, bool _add)
	{
		if (!m_showCreases || m_showAllCreases || m_creases.empty())
		{
			return;
		}
		const cinolib::DrawablePolygonmesh<>& mesh{ m_targetWidget.meshForDisplay() };
		const Vec untransfPoint{ mesh.transform.inverse() * _point };
		EdgeChain& crease{ m_creases[m_currentCrease].target };
		const bool has1{ crease.size() > 0 }, has2{ crease.size() > 1 }, has3{ crease.size() > 2 };
		const I lastI{ crease.size() - 1 };
		const std::vector<Id> endEids{ has1 ? has2 ? std::vector<Id>{crease[0], crease[lastI]} : std::vector<Id>{ crease[0] } : std::vector<Id>{} };
		const bool closed{ has3 && mesh.edges_are_adjacent(endEids[0], endEids[1]) };
		if (_add)
		{
			if (closed)
			{
				return;
			}
			Id closestEid{ noId };
			if (!has1)
			{
				closestEid = mesh.pick_edge(untransfPoint);
			}
			else
			{
				Real closestDist{ std::numeric_limits<Real>::infinity() };
				for (const Id endEid : endEids)
				{
					for (const Id eid : mesh.adj_e2e(endEid))
					{
						if (has2 && (eid == crease[1] || eid == crease[lastI - 1]))
						{
							continue;
						}
						const Real dist{ untransfPoint.dist_sqrd(mesh.edge_sample_at(eid, 0.5)) };
						if (dist < closestDist)
						{
							closestDist = dist;
							closestEid = eid;
						}
					}
				}
			}
			if (closestEid == noId)
			{
				return;
			}
			if (has2)
			{
				std::set<Id> invalidVids;
				if (has3)
				{
					for (I i{ 1 }; i < lastI; i++)
					{
						const Id eid{ crease[i] };
						invalidVids.insert(mesh.edge_vert_id(eid, 0));
						invalidVids.insert(mesh.edge_vert_id(eid, 1));
					}
				}
				else
				{
					invalidVids.insert(mesh.vert_shared(endEids[0], endEids[1]));
				}
				if (invalidVids.contains(mesh.edge_vert_id(closestEid, 0)) || invalidVids.contains(mesh.edge_vert_id(closestEid, 1)))
				{
					return;
				}
			}
			crease.push_back(closestEid);
			m_targetWidget.paintEdge(closestEid, cinolib::Color::hsv2rgb(static_cast<float>(m_currentCrease) / static_cast<float>(m_creases.size()), 1.0f, 1.0f));
		}
		else
		{
			const std::vector<Id>& candidates{ closed ? crease : endEids };
			Real closestDist{ std::numeric_limits<Real>::infinity() };
			Id closestEid{ noId };
			for (const Id eid : candidates)
			{
				const Real dist{ untransfPoint.dist_sqrd(mesh.edge_sample_at(eid, 0.5)) };
				if (dist < closestDist)
				{
					closestDist = dist;
					closestEid = eid;
				}
			}
			if (closestEid == noId)
			{
				return;
			}
			const I i{ static_cast<I>(std::find(crease.begin(), crease.end(), closestEid) - crease.begin()) };
			const I nextI{ (i + 1) % crease.size() };
			std::rotate(crease.begin(), crease.begin() + nextI, crease.end());
			crease.pop_back();
			m_targetWidget.paintEdge(closestEid, m_targetWidget.edgeColor());
		}
	}

	void Projection::clearCreases()
	{
		m_currentCrease = 0;
		updateMeshEdges(0, m_creases.size(), false);
		m_creases.clear();
	}

	void Projection::addCrease()
	{
		m_creases.push_back({});
		updateMeshEdges(0, m_creases.size(), true);
	}

	void Projection::removeCrease(I _index)
	{
		updateMeshEdges(_index, _index + 1, false);
		std::swap(m_creases[_index], m_creases[m_creases.size() - 1]);
		m_creases.pop_back();
		if (_index == m_currentCrease)
		{
			m_currentCrease = 0;
		}
		updateMeshEdges(0, m_creases.size(), false);
	}

	void Projection::draw()
	{
		static constexpr auto tweak{ [](Algorithms::Projection::Tweak& _tweak, const char* _header) {
			ImGui::PushID(&_tweak);
			ImGui::Text("%s", _header);
			float min{static_cast<float>(_tweak.min())};
			ImGui::SliderFloat("Min", &min, -2.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
			float power{static_cast<float>(_tweak.power())};
			ImGui::SliderFloat("Power", &power, 0.0f, 4.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_Logarithmic);
			_tweak = { min, power };
			ImGui::PopID();
		} };
		{
			int iterations{ static_cast<int>(m_options.iterations) };
			if (ImGui::SliderInt("Iterations", &iterations, 1, 10, "%d", ImGuiSliderFlags_AlwaysClamp))
			{
				m_options.iterations = static_cast<I>(iterations);
			}
		}
		ImGui::Spacing();
		{
			int invertMode{ static_cast<int>(m_options.invertMode) };
			if (ImGui::Combo("Invert mode", &invertMode, "Distance\0BarycentricCoords\0"))
			{
				m_options.invertMode = static_cast<Algorithms::Projection::EInvertMode>(invertMode);
			}
			int displaceMode{ static_cast<int>(m_options.displaceMode) };
			if (ImGui::Combo("Displace mode", &displaceMode, "NormDirAvgAndDirNormAvg\0NormDirAvgAndDirAvg\0DirAvg\0VertAvg\0"))
			{
				m_options.displaceMode = static_cast<Algorithms::Projection::EDisplaceMode>(displaceMode);
			}
		}
		ImGui::Spacing();
		tweak(m_options.weightTweak, "Weight factor");
		ImGui::Spacing();
		tweak(m_options.normalDotTweak, "Normal dot factor");
		ImGui::Spacing();
		tweak(m_options.unsetVertsDistWeightTweak, "Unset verts distance factor");
		ImGui::Spacing();
		{
			float advancePercentile{ static_cast<float>(m_options.advancePercentile) * 100.0f };
			if (ImGui::SliderFloat("Advance percentile", &advancePercentile, 0.0f, 100.0f, "%.2f%%", ImGuiSliderFlags_AlwaysClamp))
			{
				m_options.advancePercentile = advancePercentile / 100.0f;
			}
		}
		ImGui::Checkbox("Smooth", &m_options.smooth);
		ImGui::Spacing();
		ImGui::SetNextItemOpen(m_showCreases, ImGuiCond_Always);
		bool wasShowingCreases{ m_showCreases };
		m_showCreases = ImGui::TreeNode("Creases");
		if (wasShowingCreases != m_showCreases)
		{
			updateMeshEdges(0, m_creases.size(), m_showCreases);
		}
		if (m_showCreases)
		{
			if (ImGui::TreeNode("Auto finder"))
			{
				ImGui::SliderFloat("Angle threshold", &m_featureFinderOptions.ang_thresh_deg, 0.0f, 180.0f, "%.0f deg", ImGuiSliderFlags_AlwaysClamp);
				ImGui::Checkbox("Split", &m_featureFinderOptions.split_lines_at_high_curvature_points);
				if (ImGui::Button("Find in source"))
				{
					findCreases(true);
				}
				if (m_targetWidget.hasMesh())
				{
					ImGui::SameLine();
					if (ImGui::Button("Find in target"))
					{
						findCreases(false);
					}
				}
				ImGui::TreePop();
			}
			ImGui::Spacing();
			if (m_creases.empty())
			{
				ImGui::TextDisabled("No creases");
			}
			else
			{
				ImGui::Checkbox("Show all", &m_showAllCreases);
				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
				for (I i{}; i < m_creases.size(); i++)
				{
					ImGui::PushID(static_cast<int>(i));
					if (!m_showAllCreases)
					{
						int currentCrease = static_cast<int>(m_currentCrease);
						if (ImGui::RadioButton("", &currentCrease, static_cast<int>(i)))
						{
							updateMeshEdges(m_currentCrease, m_currentCrease + 1, false);
							m_currentCrease = static_cast<I>(currentCrease);
							updateMeshEdges(m_currentCrease, m_currentCrease + 1, true);
						}
						ImGui::SameLine();
					}
					ImVec4 color;
					ImGui::ColorConvertHSVtoRGB(static_cast<float>(i) / static_cast<float>(m_creases.size()), 1.0f, 1.0f, color.x, color.y, color.z);
					color.w = 1.0f;
					ImGui::ColorButton("##color", color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoTooltip);
					ImGui::SameLine();
					if (ImGui::Button("Remove"))
					{
						removeCrease(i);
					}
					ImGui::SameLine();
					if (ImGui::SmallButton("Source from selection"))
					{
						setSourceCreaseFromSelection(i);
					}
					ImGui::SameLine();
					if (ImGui::SmallButton("Source to selection"))
					{
						m_vertEditWidget.clear();
						std::vector<Id> vids{};
						const EdgeChain& chain{ m_creases[i].source };
						vids.reserve(chain.size() * 2);
						for (const Id eid : chain)
						{
							vids.push_back(m_mesher.mesh().edge_vert_id(eid, 0));
							vids.push_back(m_mesher.mesh().edge_vert_id(eid, 1));
						}
						m_vertEditWidget.add(vids);
					}
					if (m_targetWidget.hasMesh())
					{
						ImGui::SameLine();
						if (ImGui::SmallButton("Clear target"))
						{
							updateTargetMeshEdges(i, i + 1, false);
							m_creases[i].target.clear();
						}
						ImGui::SameLine();
						if (ImGui::SmallButton("Match source"))
						{
							matchCreases(i, i + 1, true);
						}
						ImGui::SameLine();
						if (ImGui::SmallButton("Match target"))
						{
							matchCreases(i, i + 1, false);
						}
					}
					ImGui::PopID();
				}
				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
			}
			ImGui::Spacing();
			if (ImGui::Button("Add"))
			{
				addCrease();
			}
			if (!m_creases.empty())
			{
				ImGui::SameLine();
				if (ImGui::Button("Clear"))
				{
					clearCreases();
				}
				if (m_targetWidget.hasMesh())
				{
					ImGui::SameLine();
					if (ImGui::SmallButton("Match all source"))
					{
						matchCreases(0, m_creases.size(), true);
					}
					ImGui::SameLine();
					matchCreases(0, m_creases.size(), true);
					if (ImGui::SmallButton("Match all target"))
					{
						matchCreases(0, m_creases.size(), false);
					}
				}
			}
			ImGui::TreePop();
		}
		if (m_targetWidget.hasMesh())
		{
			ImGui::Spacing();
			if (ImGui::Button("Project"))
			{
				requestProjection();
			}
			if (canReproject())
			{
				ImGui::SameLine();
				if (ImGui::Button("Reproject"))
				{
					requestReprojection();
				}
			}
		}
	}

}
