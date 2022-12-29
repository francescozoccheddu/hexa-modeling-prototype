# Issues and possible improvements
Sorted by priority:
- **\[BUG\]** `HMP::Actions::Root::~Root()` leads to `HMP::Dag::Node` double free on app exit (I think the `HMP::Dag::Node` detachment system is broken).
- **\[IMPROVEMENT\]** `HMP::Refinement::Utils::apply` should use `HMP::Refinement::Scheme::facesSurfVisIs` in place of `HMP::Refinement::Utils::weldAdjacencies_TEMP_NAIVE` (how do I rotate the scheme vertices and match the adjacent faces?).
- **\[IMPROVEMENT\]** Improve `HMP::Actions::MakeConforming` either by implementing a balancing preprocessing phase, or by defining more adapter schemes.
- **\[BUG\]** `cinolib::feature_mapping` and `HMP::Gui::Widgets::Projection::matchPaths` crash with relatively short paths.
- **\[BUG\]** `cinolib::feature_mapping` chooses the shortest path without following the edge chain.
- **\[IMPROVEMENT\]** `HMP::Actions::MakeConforming` performance can be improved a lot by keeping a queue of non-conforming refinements.
- **\[FEATURE\]** The extrude operation could automatically determine the number of parents, or at least give the user a warning in case of unintentionally duplicate vertices.
- **\[IMPROVEMENT\]** Projection feature paths, widget options and camera could be serialized too.
- **\[FEATURE\]** Perhaps pasting a subtree should not preserve the source size (or maybe the choice could be left to the user).
- **\[REFACTOR\]** `HMP::Meshing::Utils` is a dumpsite full of duplicated code. Keep the few essential primitives and throw everything else away.
- **\[REFACTOR\]** The `HMP::Gui::App` class is too big. Keep splitting it into child components.
- **\[IMPROVEMENT\]** The `HMP::Gui::Widgets::DirectVertEdit` scale and rotation implementation is a bit janky.
- **\[REFACTOR\]** All the `HMP::Meshing::Actions` could be replaced with a set of more primitive actions (`MoveVert`, `ShowElement`, `AddElements`, `WeldElements` and `ActionSequence` maybe?).
- **\[IMPROVEMENT\]** `OGDF` is overkill for what I need. Consider replacing it with a lighter implementation of the Sugiyama layout algorithm.
- **\[REFACTOR\]** Does the `core`/`gui` CMake project separation still make sense? 
- **\[FEATURE\]** Add As-Rigid-As-Possible vertex editing support.
- **\[FEATURE\]** Enable undo/redo support for every user action, not just meshing operations.