// remove controls that don't have any side-effect by existing
// it is a common pattern to "break out" a control into a separate node, and then connect it to
// any destinations, but the value-group system means we can safely remove these when they are
// types without side effects (e.g. num or midi controls, but not a graph editor)
// we can also remove "editor only" controls (such as a vector scope), but this should
// probably go in the AST lowering pass
pub fn remove_dead_controls() {}
