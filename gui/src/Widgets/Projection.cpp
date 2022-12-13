#include <HMP/Gui/Widgets/Projection.hpp>

#include <imgui.h>
#include <utility>
#include <stdexcept>
#include <HMP/Actions/Project.hpp>
#include <cinolib/export_surface.h>
#include <limits>
#include <algorithm>
#include <set>
#include <cinolib/feature_mapping.h>
#include <HMP/Projection/Utils.hpp>
#include <HMP/Gui/Utils/Controls.hpp>

namespace HMP::Gui::Widgets
{

	Projection::Projection(Widgets::Target& _targetWidget, HMP::Commander& _commander, Meshing::Mesher& _mesher, VertEdit& _vertEditWidget):
		cinolib::SideBarItem{ "Projection" }, m_targetWidget{ _targetWidget }, m_commander{ _commander }, m_mesher{ _mesher }, m_vertEditWidget{ _vertEditWidget },
		onProjectRequest{}, m_options{}, m_paths(1)
	{
		m_targetWidget.onMeshChanged += [this]() {
			clearTargetPaths(0, m_paths.size());
		};
		m_mesher.onElementAdd += [this](const Dag::Element& _element) {
			for (I i{}; i < m_paths.size(); i++)
			{
				for (const Id eid : m_paths[i].sourceEids)
				{
					if (!m_mesher.mesh().edge_is_on_srf(eid))
					{
						clearSourcePaths(i, i + 1);
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
				for (I i{}; i < m_paths.size(); i++)
				{
					for (Id& eid : m_paths[i].sourceEids)
					{
						if (eid == lastEid)
						{
							eid = removedEid;
						}
						else if (eid == removedEid)
						{
							clearSourcePaths(i, i + 1);
							break;
						}
					}
				}
			}
		};
		m_mesher.onClear += [this]() {
			clearSourcePaths(0, m_paths.size());
		};
	}

	const HMP::Projection::Options& Projection::options() const
	{
		return m_options;
	}

	void Projection::requestProjection()
	{
		if (!m_targetWidget.hasMesh())
		{
			throw std::logic_error{ "no target mesh" };
		}
		std::vector<HMP::Projection::Utils::Point> points;
		points.reserve(m_paths.size() * 2);
		std::vector<HMP::Projection::Utils::EidsPath> paths;
		paths.reserve(m_paths.size());
		const cinolib::Polygonmesh<>& targetMesh{ m_targetWidget.meshForProjection() };
		for (const HMP::Projection::Utils::EidsPath& path : m_paths)
		{
			if (path.sourceEids.empty() || path.targetEids.empty())
			{
				continue;
			}
			paths.push_back(path);
			for (const HMP::Projection::Utils::Point& point : HMP::Projection::Utils::endPoints(path, m_mesher.mesh(), targetMesh))
			{
				points.push_back(point);
			}
		}
		onProjectRequest(targetMesh, points, paths, m_options);
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

	void Projection::matchPaths(I _first, I _lastEx, bool _fromSource)
	{
		cinolib::Polygonmesh<> target{ m_targetWidget.meshForProjection() };
		const Meshing::Mesher::Mesh& source{ m_mesher.mesh() };
		clearPaths(_first, _lastEx, !_fromSource);
		std::vector<std::vector<Id>> from(_lastEx - _first), to;
		for (I i{ _first }; i < _lastEx; i++)
		{
			from[i - _first] = _fromSource
				? HMP::Projection::Utils::eidsToVidsPath(source, m_paths[i].sourceEids)
				: HMP::Projection::Utils::eidsToVidsPath(target, m_paths[i].targetEids);
		}
		std::unordered_map<Id, Id> surf2vol, vol2surf;
		cinolib::Polygonmesh<> sourceSurf{};
		cinolib::export_surface(source, sourceSurf, vol2surf, surf2vol);
		if (_fromSource)
		{
			for (auto& vids : from)
			{
				for (Id& vid : vids)
				{
					vid = vol2surf[vid];
				}
			}
			cinolib::feature_mapping(sourceSurf, from, target, to);
		}
		else
		{
			cinolib::feature_mapping(target, from, sourceSurf, to);
			for (auto& vids : to)
			{
				for (Id& vid : vids)
				{
					vid = surf2vol[vid];
				}
			}
		}
		for (I i{ _first }; i < _lastEx; i++)
		{
			if (_fromSource)
			{
				m_paths[i].targetEids = HMP::Projection::Utils::vidsToEidsPath(target, to[i - _first]);
			}
			else
			{
				m_paths[i].sourceEids = HMP::Projection::Utils::vidsToEidsPath(source, to[i - _first]);
			}
			if (m_showPaths && (m_showAllPaths || m_currentPath == i))
			{
				updateMeshEdges(i, i + 1, true, !_fromSource);
			}
		}
	}

	void Projection::clearPaths(I _first, I _lastEx, bool _source)
	{
		if (_source)
		{
			clearSourcePaths(_first, _lastEx);
		}
		else
		{
			clearTargetPaths(_first, _lastEx);
		}
	}

	void Projection::clearBothPaths(I _first, I _lastEx)
	{
		clearSourcePaths(_first, _lastEx);
		clearTargetPaths(_first, _lastEx);
	}

	void Projection::findPaths(bool _inSource)
	{
		std::vector<std::vector<Id>> network{};
		if (_inSource)
		{
			clearSourcePaths(0, m_paths.size());
			cinolib::Polygonmesh<> surface{};
			cinolib::export_surface(m_mesher.mesh(), surface);
			cinolib::feature_network(surface, network, m_featureFinderOptions);
		}
		else
		{
			clearTargetPaths(0, m_paths.size());
			cinolib::feature_network(m_targetWidget.meshForDisplay(), network, m_featureFinderOptions);
		}
		std::cout << network.size() << std::endl;
		// FIXME Why network is empty?
		// TODO Clear
	}

	void Projection::updateMeshEdges(I _first, I _lastEx, bool _show, bool _source)
	{
		if (_source)
		{
			updateSourceMeshEdges(_first, _lastEx, _show);
		}
		else
		{
			updateTargetMeshEdges(_first, _lastEx, _show);
		}
	}

	void Projection::updateBothMeshEdges(I _first, I _lastEx, bool _show)
	{
		updateSourceMeshEdges(_first, _lastEx, _show);
		updateTargetMeshEdges(_first, _lastEx, _show);
	}

	void Projection::updateSourceMeshEdges(I _first, I _lastEx, bool _show)
	{
		for (I i{ _first }; i < _lastEx; i++)
		{
			const cinolib::Color& color{ _show
				? cinolib::Color::hsv2rgb(static_cast<float>(i) / static_cast<float>(m_paths.size()), 1.0f, 1.0f)
				: m_mesher.edgeColor()
			};
			for (const Id eid : m_paths[i].sourceEids)
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
				? cinolib::Color::hsv2rgb(static_cast<float>(i) / static_cast<float>(m_paths.size()), 1.0f, 1.0f)
				: m_targetWidget.edgeColor()
			};
			for (const Id eid : m_paths[i].targetEids)
			{
				m_targetWidget.paintEdge(eid, color);
			}
		}
	}

	void Projection::clearSourcePaths(I _first, I _lastEx)
	{
		updateSourceMeshEdges(_first, _lastEx, false);
		for (I i{ _first }; i < _lastEx; i++)
		{
			m_paths[i].sourceEids.clear();
		}
	}

	void Projection::clearTargetPaths(I _first, I _lastEx)
	{
		updateTargetMeshEdges(_first, _lastEx, false);
		for (I i{ _first }; i < _lastEx; i++)
		{
			m_paths[i].targetEids.clear();
		}
	}

	void Projection::setSourcePathEdgeAtPoint(const Vec& _point, bool _add)
	{
		setPathEdgeAtPoint(_point, _add, m_mesher.mesh(), true);
	}

	void Projection::setTargetPathEdgeAtPoint(const Vec& _point, bool _add)
	{
		setPathEdgeAtPoint(m_targetWidget.meshForDisplay().transform.inverse() * _point, _add, m_targetWidget.meshForDisplay(), false);
	}

	void Projection::clearBothPaths()
	{
		m_currentPath = 0;
		updateBothMeshEdges(0, m_paths.size(), false);
		m_paths.clear();
	}

	void Projection::addPath()
	{
		m_paths.push_back({});
		if (m_showPaths)
		{
			if (m_showAllPaths)
			{
				updateBothMeshEdges(0, m_paths.size(), m_showPaths);
			}
			else if (!m_paths.empty())
			{
				updateBothMeshEdges(m_currentPath, m_currentPath + 1, m_showPaths);
			}
		}
	}

	void Projection::removePath(I _index)
	{
		updateBothMeshEdges(_index, _index + 1, false);
		if (_index + 1 != m_paths.size())
		{
			std::swap(m_paths[_index], m_paths.back());
		}
		m_paths.pop_back();
		if (_index == m_currentPath)
		{
			m_currentPath = 0;
		}
		updateBothMeshEdges(0, m_paths.size(), false);
	}

	void Projection::draw()
	{
		static constexpr auto tweak{ [](HMP::Projection::Utils::Tweak& _tweak, const char* _label) {
			ImGui::PushID(&_tweak);
			ImGui::Text("%s", _label);
			Real min{ _tweak.min() }, power{ _tweak.power() };
			Utils::Controls::sliderReal("Min", min, -2.0, 1.0);
			Utils::Controls::sliderReal("Power", power, 0.0, 4.0, true);
			_tweak = { min, power };
			ImGui::PopID();
		} };
		if (ImGui::TreeNode("Weights"))
		{
			ImGui::Spacing();
			Utils::Controls::combo("Displace mode", m_options.displaceMode, { "NormDirAvgAndDirNormAvg", "NormDirAvgAndDirAvg", "DirAvg", "VertAvg" });
			ImGui::Spacing();
			tweak(m_options.baseWeightTweak, "Base weight factor");
			Utils::Controls::combo("Mode", m_options.baseWeightMode, { "Distance", "Barycentric coords" });
			ImGui::Spacing();
			tweak(m_options.normalDotTweak, "Normal dot factor");
			ImGui::Spacing();
			ImGui::Text("Distance weight factor");
			Utils::Controls::sliderReal("Weight", m_options.distanceWeight, 0.0, 10.0);
			Utils::Controls::sliderReal("Power", m_options.distanceWeightPower, 0.0, 4.0, true);
			ImGui::Spacing();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Fill and advance"))
		{
			ImGui::Spacing();
			tweak(m_options.unsetVertsDistWeightTweak, "Fill distance factor");
			ImGui::Spacing();
			Utils::Controls::sliderPercentage("Advance percentile", m_options.advancePercentile);
			ImGui::Spacing();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Smoothing"))
		{
			ImGui::Spacing();
			Utils::Controls::sliderI("Surface iterations", m_options.smoothSurfaceIterations, 0, 10);
			Utils::Controls::sliderI("Internal iterations", m_options.smoothInternalIterations, 0, 10);
			Utils::Controls::sliderPercentage("Internal done weight", m_options.smoothInternalDoneWeight, 1.0, 10.0);
			ImGui::Spacing();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Jacobian check"))
		{
			ImGui::Spacing();
			Utils::Controls::combo("Target", m_options.jacobianCheckMode, { "None", "Surface only", "All" });
			Utils::Controls::combo("Advance mode", m_options.jacobianAdvanceMode, { "Length", "Lerp" });
			Utils::Controls::sliderI("Max tests", m_options.jacobianAdvanceMaxTests, 2, 20);
			Utils::Controls::sliderPercentage("Stop threshold", m_options.jacobianAdvanceStopThreshold, 0.01, 0.4);
			ImGui::Spacing();
			ImGui::TreePop();
		}
		ImGui::SetNextItemOpen(m_showPaths, ImGuiCond_Always);
		bool wasShowingPaths{ m_showPaths };
		m_showPaths = ImGui::TreeNode("Paths");
		if (wasShowingPaths != m_showPaths)
		{
			if (m_showAllPaths)
			{
				updateBothMeshEdges(0, m_paths.size(), m_showPaths);
			}
			else if (!m_paths.empty())
			{
				updateBothMeshEdges(m_currentPath, m_currentPath + 1, m_showPaths);
			}
		}
		if (m_showPaths)
		{
			ImGui::Spacing();
			if (ImGui::TreeNode("Auto finder"))
			{
				ImGui::SliderFloat("Angle threshold", &m_featureFinderOptions.ang_thresh_deg, 0.0f, 180.0f, "%.0f deg", ImGuiSliderFlags_AlwaysClamp);
				ImGui::Checkbox("Split", &m_featureFinderOptions.split_lines_at_high_curvature_points);
				if (ImGui::Button("Find in source"))
				{
					findPaths(true);
				}
				if (m_targetWidget.hasMesh())
				{
					ImGui::SameLine();
					if (ImGui::Button("Find in target"))
					{
						findPaths(false);
					}
				}
				ImGui::TreePop();
			}
			ImGui::Spacing();
			if (m_paths.empty())
			{
				ImGui::TextDisabled("No paths");
			}
			else
			{
				if (ImGui::Checkbox("Show all", &m_showAllPaths))
				{
					if (m_showPaths)
					{
						if (m_showAllPaths)
						{
							updateBothMeshEdges(0, m_paths.size(), true);
						}
						else if (!m_paths.empty())
						{
							updateBothMeshEdges(0, m_paths.size(), false);
							updateBothMeshEdges(m_currentPath, m_currentPath + 1, true);
						}
					}
				}
				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
				for (I i{}; i < m_paths.size(); i++)
				{
					ImGui::PushID(static_cast<int>(i));
					if (!m_showAllPaths)
					{
						int currentPath = static_cast<int>(m_currentPath);
						if (ImGui::RadioButton("", &currentPath, static_cast<int>(i)))
						{
							updateBothMeshEdges(m_currentPath, m_currentPath + 1, false);
							m_currentPath = static_cast<I>(currentPath);
							updateBothMeshEdges(m_currentPath, m_currentPath + 1, true);
						}
						ImGui::SameLine();
					}
					ImVec4 color;
					ImGui::ColorConvertHSVtoRGB(static_cast<float>(i) / static_cast<float>(m_paths.size()), 1.0f, 1.0f, color.x, color.y, color.z);
					color.w = 1.0f;
					ImGui::ColorButton("##color", color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoTooltip);
					ImGui::SameLine();
					if (ImGui::Button("Remove"))
					{
						removePath(i);
					}
					ImGui::SameLine();
					if (Utils::Controls::disabledSmallButton("Clear source", !m_paths[i].sourceEids.empty()))
					{
						clearSourcePaths(i, i + 1);
					}
					if (m_targetWidget.hasMesh())
					{
						ImGui::SameLine();
						if (Utils::Controls::disabledSmallButton("Clear target", !m_paths[i].targetEids.empty()))
						{
							clearTargetPaths(i, i + 1);
						}
						ImGui::SameLine();
						if (Utils::Controls::disabledSmallButton("Match source", !m_paths[i].targetEids.empty()))
						{
							matchPaths(i, i + 1, false);
						}
						ImGui::SameLine();
						if (Utils::Controls::disabledSmallButton("Match target", !m_paths[i].sourceEids.empty()))
						{
							matchPaths(i, i + 1, true);
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
				addPath();
			}
			if (!m_paths.empty())
			{
				ImGui::SameLine();
				if (ImGui::Button("Clear"))
				{
					clearBothPaths();
				}
				if (m_targetWidget.hasMesh())
				{
					ImGui::SameLine();
					if (ImGui::SmallButton("Match all source"))
					{
						matchPaths(0, m_paths.size(), false);
					}
					ImGui::SameLine();
					if (ImGui::SmallButton("Match all target"))
					{
						matchPaths(0, m_paths.size(), true);
					}
				}
			}
			ImGui::Spacing();
			ImGui::TreePop();
		}
		Utils::Controls::sliderI("Iterations", m_options.iterations, 1, 10);
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
