use crate::mir;

// removes dead sockets from all nodes in the provided surfaces
// sockets are 'dead' if they're in a value group that has no other sockets connected
pub fn remove_dead_sockets(_surfaces: &mut [mir::Surface]) {
    unimplemented!()
}

/*struct DeadSocketRemover<'a> {
    surfaces: &'a mut [mir::Surface],
}

impl<'a> DeadSocketRemover<'a> {
    pub fn new(surfaces: &'a mut [mir::Surface]) -> Self {
        DeadSocketRemover { surfaces }
    }

    pub fn remove_dead_sockets(&mut self) {
        // in order to determine which sockets we can remove, we need to take the following into account:
        //  1. sockets can't be removed from custom nodes, as they map 1:1 to controls
        //  2. the number of references to a value group by the nodes in the group - if the group
        //     is exposed, this counts as a reference and we can't remove any sockets referencing
        //     the group.
        //  3. other places where the node with the socket is used, and if those other places
        //     also have the socket marked as dead - we can *only* remove sockets if they're dead
        //     at all places (since removal affects the node MIR object)
        //  4. the node's value groups `exposer` indexes need to be changed
    }
}*/
