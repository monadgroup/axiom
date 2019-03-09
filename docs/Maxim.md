# Maxim

Maxim is a simple scripting language that allows various low-level DSP concepts and operations to be represented succinctly. In the editor, Maxim can be used by right-clicking a surface and selecting the option to create a "Custom Node". Double-clicking on this node allows entering code.

Every single built-in module is built with custom nodes (or custom nodes inside groups), meaning they all use Maxim. As a result, you can easily go into any module and edit to your hearts content.

## Language

The Maxim language is very simple. If you've ever programmed before, you'll see some similar constructs come up, like variables and functions. If not, don't worry, the language is easy to understand.

### Example 1: adding two numbers together

Let's start by creating a node that adds numbers together. In a new Custom Node, enter the following code:

```
out:num = a:num + b:num
```

Here we're "declaring" three controls: `out`, `a` and `b`. These are all `num` controls, which means they'll appear as knobs on the node. We take the value of the `a` and `b` controls, add them together, and set the `out` control to that value. There are also some other controls like `:midi` (contains a list of MIDI events) and `:graph` (a graph editor), which you can read more about in the [Language Reference](MaximReference.md#Controls).

### Example 2: complex calculations

We can use the normal rules of mathematics to do more complicated things, like raising `a` to the power of `b + 1`:

```
out:num = a:num ^ (b:num + 1)
```

### Example 3: using variables

What if we have a really long chain of operators and want to split things over multiple lines to read? For that we can use variables:

```
offsetVal = freq:num ^ (val:num + 1)
normalizedVal = offsetVal * scale:num
out:num = normalizedVal * in:num
```

### Example 4: comments

We can put comments in our code, which are ignored by the language and only serve to tell you, or other humans reading the code, what's going on:

```
# calculate the flibbidy floop of the foobanger
out:num = freq:num ^ (val:num + 1) * scale:num * in:num

/*
# only goes until the end of the line
/* and */ can comment out multiple lines
*/
```

### Example 5: functions

As the examples above show, we can use math operations to do calculations. But Maxim also exposes various "functions" which allow us to do some more complicated things. Let's say we want to find the bigger of two numbers, we can do that with the `max` function:

```
out:num = max(a:num, b:num)
```

The `sinOsc` function oscillates in a sine wave at a certain frequency, returning a value between -1 and 1:

```
osc = sinOsc(440 Hz)
out:num = osc * 0.5 + 0.5 # remap from [-1..1] to [0..1]
```

The `svFilter` applies a state-variable filter to the input, and returns all of the filtered values:

```
squareWave = sqrOsc(440 Hz)
(high:num, low:num, band:num, notch:num) = svFilter(squareWave, 500 Hz, 2 Q)
```

There are many more functions available, see the [Language Reference](MaximReference.md#Functions) for a full list.

### Example 6: global variables

Maxim provides a few global variables for common calculations. You can use these just like you would any other variable:

 - `SAMPLE_RATE` is the current sample rate (normally 44100)
 - `BPM` is the BPM of the project. In the standalone editor, this is always 60 beats. In the plugins, it's the same as your project's BPM.
 - `PI` is the mathematical pi constant (i.e 3.1415...)
 - `E` is the mathematical e constant (i.e 2.7182...)

### Example 7: forms

In Maxim (and Axiom in general), each number has an implicit "form" - the type of value stored in the number. For example, by default a number has the `none` form, but numbers can also be `freq` (frequency in Hertz), `sec` (time in seconds), `q` (Q/resonance for filters), and many more (see the [Language Reference](MaximReference.md#Forms) for the full list).

When we write a number, we can explicitly set a form. For example, try entering the code:

```
out:num = 440 Hz
```

You will see that the `out` control is shown as a frequency in the editor. We can even add magnitude specifiers, like this:

```
out:num = 440 kHz
# is the same as writing out:num = 440000 kHz
```

### Example 8: conversions

Maxim provides a bunch of pre-defined conversions between numbers of different forms, which work similarly to functions. For example, we could make a node with this code:

```
out:num = [freq] in:num
```

This will convert the number put into the `in` control into a frequency. If you put a number that can be converted to a frequency (e.g. a note) into the control, `out` will contain the frequency version of it. If there's no defined conversion, the value will just pass through.

This is very useful for calling functions that expect certain types of input numbers, since those functions don't do conversions themselves. For example, if we wanted to make a sawtooth oscillator node, we could write it like this:

```
out:num = sawOsc([freq] freq:num)
```

Let's say we have a function that expects an input in seconds, but we want to beat-sync it. We can do that like this:

```
beats = 8 beats
out:num = someFunc([secs] beats)
```

The conversion will use the current BPM (which can be accessed with the global `BPM` variable).

You can find a full list of the conversions provided in the [Language Reference](MaximReference.md#Conversions).