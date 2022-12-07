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

	Projection::Projection(Widgets::Target& _targetWidget, HMP::Commander& _commander, Meshing::Mesher& _mesher, VertEdit& _vertEditWidget):
		cinolib::SideBarItem{ "Projection" }, m_targetWidget{ _targetWidget }, m_commander{ _commander }, m_mesher{ _mesher }, m_vertEditWidget{ _vertEditWidget },
		onProjectRequest{}, m_options{}
	{
		m_targetWidget.onMeshChanged += [this]() {
			clearTargetCreases(0, m_creases.size());
		};
		m_mesher.onElementAdd += [this](const Dag::Element& _element) {
			for (I i{}; i < m_creases.size(); i++)
			{
				for (const Id eid : m_creases[i].source)
				{
					if (!m_mesher.mesh().edge_is_on_srf(eid))
					{
						clearSourceCreases(i, i + 1);
						break;
					}
				}
			}
		};
		m_mesher.onElementRemove += [this](const Dag::Element& _element, const Meshing::Mesher::RemovedIds& _removedIds) {
			Id lastEid{ m_mesher.mesh().num_edges() };
			for (const Id removedEid : _removedIds.eids)
			{
				lastEid--;
				for (I i{}; i < m_creases.size(); i++)
				{
					for (Id& eid : m_creases[i].source)
					{
						if (eid == lastEid)
						{
							eid = removedEid;
						}
						else if (eid == removedEid)
						{
							clearSourceCreases(i, i + 1);
							break;
						}
					}
				}
			}
		};
		m_mesher.onClear += [this]() {
			clearSourceCreases(0, m_creases.size());
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
			clearSourceCreases(0, m_creases.size());
			cinolib::Polygonmesh<> surface{};
			cinolib::export_surface(m_mesher.mesh(), surface);
			cinolib::feature_network(surface, network, m_featureFinderOptions);
		}
		else
		{
			clearTargetCreases(0, m_creases.size());
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

	void Projection::clearSourceCreases(I _first, I _lastEx)
	{
		updateSourceMeshEdges(_first, _lastEx, false);
		for (I i{ _first }; i < _lastEx; i++)
		{
			m_creases[i].source.clear();
		}
	}

	void Projection::clearTargetCreases(I _first, I _lastEx)
	{
		updateTargetMeshEdges(_first, _lastEx, false);
		for (I i{ _first }; i < _lastEx; i++)
		{
			m_creases[i].target.clear();
		}
	}

	void Projection::setSourceCreaseEdgeAtPoint(const Vec& _point, bool _add)
	{
		setCreaseEdgeAtPoint(_point, _add, m_mesher.mesh(), true);
	}

	void Projection::setTargetCreaseEdgeAtPoint(const Vec& _point, bool _add)
	{
		setCreaseEdgeAtPoint(m_targetWidget.meshForDisplay().transform.inverse() * _point, _add, m_targetWidget.meshForDisplay(), false);
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
					const bool sourceEmpty = m_creases[i].source.empty();
					if (sourceEmpty)
					{
						ImGui::BeginDisabled();
					}
					if (ImGui::SmallButton("Clear source"))
					{
						clearSourceCreases(i, i + 1);
					}
					if (sourceEmpty)
					{
						ImGui::EndDisabled();
					}
					if (m_targetWidget.hasMesh())
					{
						ImGui::SameLine();
						const bool targetEmpty = m_creases[i].target.empty();
						if (targetEmpty)
						{
							ImGui::BeginDisabled();
						}
						if (ImGui::SmallButton("Clear target"))
						{
							clearTargetCreases(i, i + 1);
						}
						if (targetEmpty)
						{
							ImGui::EndDisabled();
						}
						ImGui::SameLine();
						if (targetEmpty) ImGui::BeginDisabled();
						if (ImGui::SmallButton("Match source"))
						{
							matchCreases(i, i + 1, true);
						}
						if (targetEmpty) ImGui::EndDisabled();
						ImGui::SameLine();
						if (sourceEmpty) ImGui::BeginDisabled();
						if (ImGui::SmallButton("Match target"))
						{
							matchCreases(i, i + 1, false);
						}
						if (sourceEmpty) ImGui::EndDisabled();
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
