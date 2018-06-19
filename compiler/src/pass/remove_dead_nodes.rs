use mir;

// removes dead nodes, i.e nodes whose outputs never make it out of the surface
// combined with dead code removal, this will entirely remove nodes just made of controls (e.g.
// the "Knob" module)
pub fn remove_dead_nodes() {}
