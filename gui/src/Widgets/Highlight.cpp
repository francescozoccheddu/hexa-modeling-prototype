#include <HMP/Gui/Widgets/Highlight.hpp>

#include <HMP/Meshing/types.hpp>
#include <imgui.h>
#include <cinolib/deg_rad.h>
#include <cinolib/gl/glcanvas.h>
#include <cinolib/color.h>
#include <HMP/Gui/Utils/Controls.hpp>
#include <HMP/Gui/Utils/Drawing.hpp>
#include <cmath>
#include <array>
#include <utility>
#include <algorithm>
#include <HMP/Gui/themer.hpp>
#include <HMP/Gui/App.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <HMP/Gui/Utils/HrDescriptions.hpp>

namespace HMP::Gui::Widgets
{

	void Highlight::drawCanvas()
	{
		const float
			smallVertRadius{ 4.0f * themer->ovScale },
			vertRadius{ 6.0f * themer->ovScale },
			lineThickness{ 1.5f * themer->ovScale },
			boldLineThickness{ 3.5f * themer->ovScale },
			lineSpacing{ (10.0f * themer->ovScale + 10.0f) / 2.0f };
		ImDrawList& drawList{ *ImGui::GetWindowDrawList() };
		using namespace Utils::Drawing;
		if (app().copiedElement && !app().mouse().element)
		{
			const Id cPid{ app().copiedElement->pid };
			const Dag::Extrude& extrude{ app().copiedElement->parents.single().as<Dag::Extrude>() };
			const auto cPidCenter2d{ Utils::Drawing::project(app().canvas, app().mesher.mesh().poly_centroid(cPid)) };
			if (cPidCenter2d)
			{
				for (const auto& [parent, fi] : extrude.parents.zip(extrude.fis))
				{
					const QuadVertIds parentFidVids{ Meshing::Utils::fiVids(parent.vids, fi) };
					const Vec parentFidCenter{ Meshing::Utils::centroid(Meshing::Utils::verts(app().mesher.mesh(), parentFidVids)) };
					const auto parentFidCenter2d{ Utils::Drawing::project(app().canvas, parentFidCenter) };
					if (parentFidCenter2d)
					{
						dashedLine(drawList, { *parentFidCenter2d, *cPidCenter2d }, themer->ovMut, lineThickness, lineSpacing);
					}
				}
			}
			circle(drawList, cPidCenter2d, smallVertRadius, app().mouse().element == app().copiedElement ? themer->ovHi : themer->ovMut, lineThickness);
			const Dag::Element& firstParent{ extrude.parents.first() };
			const Id firstVid{ firstParent.vids[extrude.firstVi] };
			const QuadVertIds firstParentVids{ Meshing::Utils::align(Meshing::Utils::fiVids(firstParent.vids, extrude.fis[0]), firstVid, extrude.clockwise) };
			const auto eid2d{ Utils::Drawing::project(app().canvas, Meshing::Utils::verts(app().mesher.mesh(), EdgeVertIds{ firstParentVids[0], firstParentVids[1] })) };
			dashedLine(drawList, eid2d, themer->ovMut, boldLineThickness, lineSpacing);
			if (eid2d)
			{
				circleFilled(drawList, (*eid2d)[0], vertRadius, themer->ovMut);
			}
		}
		if (app().mouse().element)
		{
			for (I i{}; i < 6; i++)
			{
				const I fi{ (i + 1 + app().mouse().fi) % 6};
				const QuadVerts fiVerts{ Meshing::Utils::verts(app().mesher.mesh(), Meshing::Utils::fiVids(app().mouse().element->vids, fi)) };
				const auto fiVerts2d{ Utils::Drawing::project(app().canvas, fiVerts) };
				quadFilled(drawList, fiVerts2d, fi == app().mouse().fi ? themer->ovFaceHi : themer->ovPolyHi);
			}
			const auto hPidCenter2d{ Utils::Drawing::project(app().canvas, app().mesher.mesh().poly_centroid(app().mouse().pid)) };
			if (hPidCenter2d)
			{
				for (const Id adjPid : app().mesher.mesh().adj_p2p(app().mouse().pid))
				{
					const Id adjFid{ static_cast<Id>(app().mesher.mesh().poly_shared_face(app().mouse().pid, adjPid)) };
					const auto adjFidCenter2d{ Utils::Drawing::project(app().canvas, app().mesher.mesh().face_centroid(adjFid)) };
					const auto adjPidCenter2d{ Utils::Drawing::project(app().canvas, app().mesher.mesh().poly_centroid(adjPid)) };
					const ImU32 adjColorU32{ app().mesher.shown(adjPid) ? themer->ovHi : themer->ovMut };
					circle(drawList, adjPidCenter2d, smallVertRadius, adjColorU32, lineThickness);
					if (adjFidCenter2d)
					{
						dashedLine(drawList, { *adjFidCenter2d, *hPidCenter2d }, adjColorU32, lineThickness, lineSpacing);
					}
				}
				circle(drawList, *hPidCenter2d, smallVertRadius, themer->ovHi, lineThickness);
			}
			for (const Id adjEid : app().mesher.mesh().adj_f2e(app().mouse().fid))
			{
				const auto adjEid2d{ Utils::Drawing::project(app().canvas, Meshing::Utils::verts(app().mesher.mesh(), Meshing::Utils::eidVids(app().mesher.mesh(), adjEid))) };
				dashedLine(drawList, adjEid2d, adjEid == app().mouse().eid ? themer->ovHi : themer->ovMut, boldLineThickness, lineSpacing);
			}
			const auto hVert2d{ Utils::Drawing::project(app().canvas, app().mesher.mesh().vert(app().mouse().vid)) };
			circleFilled(drawList, hVert2d, vertRadius, themer->ovHi);
		}
		if (app().mouse().element)
		{
			std::ostringstream stream{};
			stream
				<< "Hovering "
				<< Utils::HrDescriptions::name(*app().mouse().element, app().dagNamer)
				<< " ("
				<< "face " << app().mouse().fi
				<< ", edge " << app().mouse().ei
				<< ", vert " << app().mouse().vi
				<< ")";
			ImGui::TextColored(toImVec4(themer->ovMut), "%s", stream.str().c_str());
		}
		if (app().copiedElement)
		{
			std::ostringstream stream{};
			stream
				<< "Copied"
				<< " " << Utils::HrDescriptions::name(*app().copiedElement, app().dagNamer);
			ImGui::TextColored(toImVec4(themer->ovMut), "%s", stream.str().c_str());
		}
	}

}