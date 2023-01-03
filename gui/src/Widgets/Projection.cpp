#include <HMP/Gui/Widgets/Projection.hpp>

#include <utility>
#include <cassert>
#include <HMP/Actions/Project.hpp>
#include <cinolib/export_surface.h>
#include <cinolib/deg_rad.h>
#include <limits>
#include <algorithm>
#include <set>
#include <cinolib/feature_mapping.h>
#include <HMP/Projection/Utils.hpp>
#include <HMP/Gui/Utils/Controls.hpp>
#include <HMP/Gui/Utils/Drawing.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <unordered_set>
#include <cpputils/range/of.hpp>
#include <HMP/Gui/themer.hpp>


namespace HMP::Gui::Widgets
{

	Projection::Projection(const Widgets::Target& _targetWidget, HMP::Commander& _commander, const Meshing::Mesher& _mesher):
		cinolib::SideBarItem{ "Projection" }, m_targetWidget{ _targetWidget }, m_commander{ _commander }, m_mesher{ _mesher },
		m_options{}, m_paths(1), m_featureFinderOptions{}, m_usePathAsCrease{ false }, m_featureFinderCreaseAngle{ 60.0f },
		m_showPaths{ false }, m_showAllPaths{ false }, m_currentPath{}, onProjectRequest{}
	{
		m_targetWidget.onMeshChanged += [this]() {
			for (auto& path : m_paths)
			{
				path.targetEids.clear();
			}
		};
		m_mesher.onAdded += [this](const Meshing::Mesher::State&) {
			for (I i{}; i < m_paths.size(); i++)
			{
				for (const Id eid : m_paths[i].sourceEids)
				{
					if (!m_mesher.mesh().edge_is_visible(eid))
					{
						m_paths[i].sourceEids.clear();
						break;
					}
				}
			}
		};
		m_mesher.onElementVisibilityChanged += [this](const Dag::Element& _element, bool) {
			const std::vector<Id> removedEids{ m_mesher.mesh().poly_dangling_eids(_element.pid) };
			const std::unordered_set<Id> removedEidsSet{ removedEids.begin(), removedEids.end() };
			for (I i{}; i < m_paths.size(); i++)
			{
				for (Id& eid : m_paths[i].sourceEids)
				{
					if (removedEidsSet.contains(eid))
					{
						m_paths[i].sourceEids.clear();
						break;
					}
				}
			}
		};
		m_mesher.onRestored += [this](const Meshing::Mesher::State&) {
			const Id eidCount{ m_mesher.mesh().num_edges() };
			for (I i{}; i < m_paths.size(); i++)
			{
				for (const Id eid : m_paths[i].sourceEids)
				{
					if (eid >= eidCount)
					{
						m_paths[i].sourceEids.clear();
						break;
					}
				}
			}
		};
	}

	const HMP::Projection::Options& Projection::options() const
	{
		return m_options;
	}

	void Projection::requestProjection()
	{
		assert(m_targetWidget.hasMesh());
		std::vector<Point> points;
		points.reserve(m_paths.size() * 2);
		std::vector<EidsPath> paths;
		paths.reserve(m_paths.size());
		const cinolib::Polygonmesh<>& targetMesh{ m_targetWidget.meshForProjection() };
		for (const EidsPath& path : m_paths)
		{
			if (path.sourceEids.empty() || path.targetEids.empty())
			{
				continue;
			}
			paths.push_back(path);
			for (const Point& point : HMP::Projection::Utils::endPoints(path, m_mesher.mesh(), targetMesh))
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
		assert(canReproject());
		m_commander.undo();
		requestProjection();
	}

	void Projection::matchPaths(I _first, I _lastEx, bool _fromSource)
	{
		cinolib::Polygonmesh<> target{ m_targetWidget.meshForProjection() };
		const Meshing::Mesher::Mesh& source{ m_mesher.mesh() };
		for (I i{ _first }; i < _lastEx; i++)
		{
			m_paths[i].eids(!_fromSource).clear();
		}
		std::vector<std::vector<Id>> from(_lastEx - _first), to;
		for (I i{ _first }; i < _lastEx; i++)
		{
			from[i - _first] = _fromSource
				? HMP::Projection::Utils::eidsToVidsPath(source, m_paths[i].sourceEids)
				: HMP::Projection::Utils::eidsToVidsPath(target, m_paths[i].targetEids);
		}
		std::unordered_map<Id, Id> surf2vol, vol2surf;
		cinolib::Polygonmesh<> sourceSurf{};
		cinolib::export_surface(source, sourceSurf, vol2surf, surf2vol, false);
		if (_fromSource)
		{
			for (auto& vids : from)
			{
				for (Id& vid : vids)
				{
					vid = vol2surf[vid];
				}
			}
			if (!cinolib::feature_mapping(sourceSurf, from, target, to))
			{
				return;
			}
		}
		else
		{
			if (!cinolib::feature_mapping(target, from, sourceSurf, to))
			{
				return;
			}
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
		}
	}

	void Projection::findPaths(bool _inSource)
	{
		std::vector<std::vector<Id>> network{};
		cinolib::Polygonmesh<> mesh;
		std::unordered_map<Id, Id> surf2vol;
		if (_inSource)
		{
			std::unordered_map<Id, Id> vol2surf;
			cinolib::export_surface(m_mesher.mesh(), mesh, vol2surf, surf2vol, false);
			if (m_usePathAsCrease)
			{
				for (const EidsPath path : m_paths)
				{
					for (const Id eid : path.sourceEids)
					{
						const Id surfEid{ static_cast<Id>(mesh.edge_id(
							vol2surf.at(m_mesher.mesh().edge_vert_id(eid, 0)),
							vol2surf.at(m_mesher.mesh().edge_vert_id(eid, 1))
						)) };
						mesh.edge_data(surfEid).flags[cinolib::CREASE] = true;
					}
				}
			}
		}
		else
		{
			mesh = m_targetWidget.meshForDisplay();
			if (m_usePathAsCrease)
			{
				for (const EidsPath path : m_paths)
				{
					for (const Id eid : path.targetEids)
					{
						mesh.edge_data(eid).flags[cinolib::CREASE] = true;
					}
				}
			}
		}
		if (!m_usePathAsCrease)
		{
			mesh.edge_mark_sharp_creases(static_cast<float>(cinolib::to_rad(static_cast<double>(m_featureFinderCreaseAngle))));
		}
		cinolib::feature_network(mesh, network, m_featureFinderOptions);
		m_paths.resize(std::max(m_paths.size(), network.size()));
		network.resize(m_paths.size());
		for (const auto& [path, featVids] : cpputils::range::zip(m_paths, network))
		{
			if (_inSource)
			{
				for (Id& vid : featVids)
				{
					vid = surf2vol.at(vid);
				}
				path.sourceEids = HMP::Projection::Utils::vidsToEidsPath(m_mesher.mesh(), featVids);
			}
			else
			{
				path.targetEids = HMP::Projection::Utils::vidsToEidsPath(mesh, featVids);
			}
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

	ImVec4 Projection::pathColor(I _path) const
	{
		ImVec4 color;
		ImGui::ColorConvertHSVtoRGB(static_cast<float>(_path) / static_cast<float>(m_paths.size()), themer->ovPathSat, themer->ovPathVal, color.x, color.y, color.z);
		color.w = 1.0f;
		return color;
	}

	void Projection::removePath(I _path)
	{
		if (_path + 1 != m_paths.size())
		{
			std::swap(m_paths[_path], m_paths.back());
		}
		m_paths.pop_back();
		if (_path == m_currentPath)
		{
			m_currentPath = 0;
		}
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
			ImGui::Text("Advance");
			Utils::Controls::sliderPercentage("Advance percentile", m_options.advancePercentile);
			ImGui::Spacing();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Smoothing"))
		{
			ImGui::Spacing();
			Utils::Controls::sliderI("Surface iterations", m_options.smoothSurfaceIterations, 0, 20);
			Utils::Controls::sliderI("Internal iterations", m_options.smoothInternalIterations, 0, 20);
			Utils::Controls::sliderPercentage("Internal done weight", m_options.smoothInternalDoneWeight, 0.25, 4.0);
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
		m_showPaths = ImGui::TreeNode("Paths");
		if (m_showPaths)
		{
			ImGui::Spacing();
			if (ImGui::Button("Add"))
			{
				m_paths.push_back({});
			}
			ImGui::SameLine();
			if (Utils::Controls::disabledButton("Clear", !m_paths.empty()))
			{
				m_currentPath = 0;
				m_paths.clear();
			}
			ImGui::SameLine();
			if (Utils::Controls::disabledButton("Clear empty", !cpputils::range::of(m_paths).filter([](const EidsPath& _path) { return _path.empty(); }).empty()))
			{
				for (I i{}; i < m_paths.size();)
				{
					if (m_paths[i].empty())
					{
						removePath(i);
					}
					else
					{
						++i;
					}
				}
			}
			ImGui::SameLine();
			ImGui::Checkbox("Show all", &m_showAllPaths);
			ImGui::SameLine();
			ImGui::TextColored(m_paths.empty() ? themer->sbWarn : themer->sbOk, "%u paths", static_cast<unsigned int>(m_paths.size()));
			ImGui::Spacing();
			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{ 4.0f, 4.0f });
			ImGui::BeginTable(
				"List",
				8,
				ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersOuter,
				{ ImGui::GetContentRegionAvail().x, 170.0f * themer->sbScale }
			);
			for (I i{}; i < m_paths.size(); i++)
			{
				ImGui::PushID(static_cast<int>(i));
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				if (m_showAllPaths) { ImGui::BeginDisabled(); }
				int currentPath = m_showAllPaths ? -1 : static_cast<int>(m_currentPath);
				if (ImGui::RadioButton("", &currentPath, static_cast<int>(i)))
				{
					m_currentPath = static_cast<I>(currentPath);
				}
				if (m_showAllPaths) { ImGui::EndDisabled(); }
				ImGui::TableNextColumn();
				ImGui::ColorButton("##color", pathColor(i), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoTooltip);
				ImGui::TableNextColumn();
				ImGui::TextColored(m_paths[i].empty() ? themer->sbWarn : themer->sbOk, "[%u/%u]", static_cast<unsigned int>(m_paths[i].sourceEids.size()), static_cast<unsigned int>(m_paths[i].targetEids.size()));
				ImGui::TableNextColumn();
				bool removed{ false };
				if (ImGui::Button("Remove"))
				{
					removePath(i);
					removed = true;
				}
				ImGui::TableNextColumn();
				if (Utils::Controls::disabledButton("Clear source", !removed && !m_paths[i].sourceEids.empty()))
				{
					m_paths[i].sourceEids.clear();
				}
				if (!m_targetWidget.hasMesh()) { ImGui::BeginDisabled(); }
				ImGui::TableNextColumn();
				if (Utils::Controls::disabledButton("Clear target", !removed && !m_paths[i].targetEids.empty()))
				{
					m_paths[i].targetEids.clear();
				}
				ImGui::TableNextColumn();
				if (Utils::Controls::disabledButton("Match source", !removed && !m_paths[i].targetEids.empty()))
				{
					matchPaths(i, i + 1, false);
				}
				ImGui::TableNextColumn();
				if (Utils::Controls::disabledButton("Match target", !removed && !m_paths[i].sourceEids.empty()))
				{
					matchPaths(i, i + 1, true);
				}
				if (!m_targetWidget.hasMesh()) { ImGui::EndDisabled(); }
				ImGui::PopID();
			}
			ImGui::EndTable();
			ImGui::PopStyleVar();
			ImGui::Spacing();
			if (m_paths.empty()) { ImGui::BeginDisabled(); }
			if (ImGui::SmallButton("Sort by source length"))
			{
				std::sort(m_paths.begin(), m_paths.end(), [](const EidsPath& _a, const EidsPath& _b) {
					return _a.sourceEids.size() < _b.sourceEids.size();
				});
			}
			if (m_targetWidget.hasMesh())
			{
				ImGui::SameLine();
				if (ImGui::SmallButton("Sort by target length"))
				{
					std::sort(m_paths.begin(), m_paths.end(), [](const EidsPath& _a, const EidsPath& _b) {
						return _a.targetEids.size() < _b.targetEids.size();
					});
				}
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
			if (m_paths.empty()) { ImGui::EndDisabled(); }
			ImGui::Spacing();
			if (ImGui::TreeNode("Auto finder"))
			{
				ImGui::Spacing();
				ImGui::Checkbox("Use path as crease", &m_usePathAsCrease);
				if (!m_usePathAsCrease)
				{
					ImGui::SliderFloat("Crease angle threshold", &m_featureFinderCreaseAngle, 0, 90, "%.3f deg", ImGuiSliderFlags_AlwaysClamp);
				}
				ImGui::Spacing();
				ImGui::Checkbox("Split", &m_featureFinderOptions.split_lines_at_high_curvature_points);
				if (m_featureFinderOptions.split_lines_at_high_curvature_points)
				{
					ImGui::SliderFloat("Split angle threshold", &m_featureFinderOptions.ang_thresh_deg, 0.0f, 180.0f, "%.0f deg", ImGuiSliderFlags_AlwaysClamp);
				}
				ImGui::Spacing();
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
				ImGui::Spacing();
				ImGui::TreePop();
			}
			ImGui::Spacing();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Project"))
		{
			ImGui::Spacing();
			Utils::Controls::sliderI("Iterations", m_options.iterations, 1, 20);
			if (m_targetWidget.hasMesh())
			{
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
			ImGui::Spacing();
			ImGui::TreePop();
		}
	}

	void Projection::draw(const cinolib::GLcanvas& _canvas)
	{
		const Meshing::Mesher::Mesh& mesh{ m_mesher.mesh() };
		const cinolib::DrawablePolygonmesh<>& targetMesh{ m_targetWidget.meshForDisplay() };
		const float lineThickness{ this->lineThickness * themer->ovScale };
		ImDrawList& drawList{ *ImGui::GetWindowDrawList() };
		const auto drawPath{ [&](const I _pathI) {
			const EidsPath& path { m_paths[_pathI] };
			const ImU32 color{ ImGui::ColorConvertFloat4ToU32(pathColor(_pathI)) };
			for (const Id eid : path.sourceEids)
			{
				const EdgeVertData<ImVec2> eid2d{ Utils::Drawing::project(_canvas, Meshing::Utils::verts(mesh, Meshing::Utils::eidVids(mesh, eid))) };
				Utils::Drawing::line(drawList, eid2d, color, lineThickness);
			}
			for (const Id eid : path.targetEids)
			{
				const std::vector<Id>& vids{ targetMesh.adj_e2v(eid) };
				const EdgeVerts verts{ cpputils::range::of(vids).map([&](const Id _vid) {
					return targetMesh.transform * targetMesh.vert(_vid);
				}).toArray<2>() };
				const EdgeVertData<ImVec2> eid2d{ Utils::Drawing::project(_canvas, verts) };
				Utils::Drawing::line(drawList, eid2d, color, lineThickness);
			}
		} };
		if (m_showPaths)
		{
			if (m_showAllPaths)
			{
				for (I i{}; i < m_paths.size(); i++)
				{
					drawPath(i);
				}
			}
			else if (!m_paths.empty())
			{
				drawPath(m_currentPath);
			}
		}
	}

}
