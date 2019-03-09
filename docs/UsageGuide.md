# Usage Guide

_A more detailed Usage Guide is coming soon, stay tuned!_

Thanks for checking out Axiom!

To start creating an instrument, take a look at the module library at the
bottom of the screen. Modules are re-usable components that you can drag and
drop into your project, and you can even save your own. Modules are made up of
one or more nodes, which are the grey boxes with knobs and sliders (controls)
on them. In order to link these nodes together (and link nodes to the output),
hold Control and drag from one control to another - this will create a
connection, which links the values of the controls together. If a control's
value is changed, any connected (directly or indirectly) controls will change.

Part of the power of Axiom comes in its scripting language, Maxim. When you
right click on a node surface, you have the option to create a "Custom Node".
By double clicking on this node, you can enter code. Check out the [documentation page
on Maxim](Maxim.md) to learn the
language. Every module is built with custom nodes - you can edit them to your
hearts content!

Axiom also provides group nodes, which group several connected nodes in a
hidden sub-surface. Double clicking a group node will open the sub-surface in a
new tab. In order to allow the controls in the group to be connected to those
out of the group, right click one in the group and click "Expose". If you go
back to the root surface, you'll now see that the group node has a control on
it!

Controls are completely configurable - double click on a control to move it
around or resize it, and right click to change it's display (knob, slider,
etc).

A third powerful feature of Axiom is "extractors". These are special types
of controls that extract any connected nodes - meaning the connected nodes
are duplicated and can operate at the same time (e.g. for polyphony).
Extractor controls appear like circles with spokes. Extracted nodes have a
thicker outline.

Got questions? Found bugs? Have feature suggestions? [Submit an issue](https://github.com/monadgroup/axiom/issues) and we'll have a look as soon as possible.