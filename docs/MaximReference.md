# Maxim Reference

This document contains a complete listing of forms, conversions, controls, and functions in the Maxim language, as well as short descriptions on how each one operates.

**Table of Contents:**

 - [Types](#types)
 - [Globals](#globals)
 - [Controls](#controls)
   - [`:num`](#num)
   - [`:midi`](#midi)
   - [`:graph`](#graph)
   - [`:num[]` and `:midi[]`](#num-and-midi)
 - [Functions](#functions)
   - [Math Functions](#math-functions)
   - [Filter Functions](#filter-functions)
   - [Oscillator Functions](#oscillator-functions)
   - [MIDI Functions](#midi-functions)
   - [Utility Functions](#utility-functions)
 - [Forms](#forms)
 - [Conversions](#conversions)
 - [Operators](#operators)
   - [Math Operators](#math-operators)
   - [Assignment Operators](#assignment-operators)
   - [Unary Operators](#unary-operators)
   - [Logical Operators](#logical-operators)
   - [Comparison Operators](#comparison-operators)
   - [Bitwise Operators](#bitwise-operators)

## Types

There are a few different types of values in Maxim. You never need to explicitly set types, this list just explains their behavior.

| Name | Description |
| --- | --- |
| `num` | A numeric value, made up of a [form](#Forms) and a left and right channel value (as 32-bit floating point values). |
| `midi` | A list of up to 16 MIDI events occurring at this moment in time. |
| `num[]` | An array of up to 32 `num` values, used for extraction/voices. |
| `midi[]` | An array of up to 32 `midi` values, used for extraction/voices. |

## Globals

The following globals are available. Globals can be accessed just like any other variable. Note that globals _always_ have a form of `[none]`.

| Name | Type | Description |
| --- | --- | --- |
| `SAMPLE_RATE` | `num` | The current sample rate in Hz - i.e the number of times a second a node's code is evaluated. This will probably usually be 44100 Hz, but it can depend on which host you're using. |
| `BPM` | `num` | The current number of beats per minute of the project - for example, 60 BPM means one beat per second. This depends on the host, however in the standalone editor it's always 60. |

## Controls

Controls are how the code in a node interacts with the rest of project, receives inputs from the user, and displays information back. Each has certain properties that can be set with the syntax `name:type.prop`, if no property is provided `.value` is used. Some controls have certain behavior, and the value that runs through connected wires depends on the type of control.

### `:num`

A plain old numeric control, which can be displayed as a plug, knob, slider, or button. This control doesn't do anything special, and wires connected will contain the value that's set.

**Properties:**

| Name | Type | Description |
| --- | --- | --- |
| `.value` | `num` | The value of the number control, which is used to display in the editor and propagated to any connected controls. |

### `:midi`

A plain old MIDI control, which is displayed as a plug. This control doesn't do anything special, and wires connected will contain the value that's set.

**Properties:**

| Name | Type | Description |
| --- | --- | --- |
| `.value` | `midi` | The value of the MIDI control, which is propagated to any connected controls. |

### `:graph`

A graph editor, useful for ADSR. A graph is made up of multiple curves, at the ends of which are "knobs" that can have a value set, and can be "tagged". A bar sweeps through the graph on repeat, and the output value is equal to the value of the current curve at the position of the bar. If the bar is at the start of a curve and the `state` property is equal to the current tagged value, the bar stops until it changes.

**Properties:**

| Name | Type | Description |
| --- | --- | --- |
| `.value` | `num` | The value at the current position of the graph. Setting this in code is a no-op. |
| `.state` | `num` | The next state that playback of the graph should stop at. |
| `.paused` | `num` | Setting to a non-zero value causes playback of the graph to pause until this value returns to 0. |
| `.time` | `num` | The current time of the graph in `[samples]`. Setting this moves the playback bar to the set value interpreted as samples (no conversion occurs). |

### `:num[]` and `:midi[]`

These are the two "extractor" controls. You can connect a non-extractor control (e.g. a `:num`) to an extractor control of the same type, which will cause the node containing the former to be "extracted" once for each value in the extractor control's value (and it will be able to read/write to that "slot" in the extractor array). This is useful for polyphony, where you want certain nodes that are independent for each voice.

| Name | Type | Description |
| --- | --- | --- |
| `.value` | `num[]` or `midi[]` | The value of the control, displayed in the GUI and propagated to any connected controls. |

## Functions

The following functions are available. Note that functions _never_ apply any conversions on their parameters, however they may interpret inputs as a certain type and may base the form of their output on one of the input forms.

All numeric functions work on both channels. For example, `cos(x)` returns a number with the left channel being the cosine of the left of x, and the right channel being the cosine of the right of x. Since MIDI values don't have two channels, functions which operate on them always use the left channel.

### Math Functions

| Signature | Description |
| --- | --- |
| `cos(x: num) -> num` | Computes the cosine of `x` as radians. Return value has the same form as the input. |
| `sin(x: num) -> num` | Computes the sine of `x` as radians. Return value has the same form as the input. |
| `tan(x: num) -> num` | Computes the tangent of `x` as radians. Return value has the same form as the input. |
| `acos(x: num) -> num` | Computes the arccos of `x`. Return value has the same form as the input. |
| `asin(x: num) -> num` | Computes the arcsine of `x`. Return value has the same form as the input. |
| `atan(x: num) -> num` | Computes the arctangent of `x`. Return value has the same form as the input. |
| `atan2(y: num, x: num) -> num` | Computes the arctangent of `y / x`. Return value has the same form as `y`. |
| `sinh(x: num) -> num` | Computes the hyperbolic sine of `x`. Return value has the same form as the input. |
| `cosh(x: num) -> num` | Computes the hyperbolic cosine of `x`. Return value has the same form as the input. |
| `tanh(x: num) -> num` | Computes the hyperbolic tangent of `x`. Return value has the same form as the input. |
| `hypot(x: num, y: num) -> num` | Computes the length of the hypotenuse with width `x` and height `y`. Return value has the same form as `x`. |
| `log(x: num) -> num` | Computes the base-e logarithm of `x`. Return value has the same form as the input. |
| `log2(x: num) -> num` | Computes the base-2 logarithm of `x`. Return value has the same form as the input. |
| `log10(x: num) -> num` | Computes the base-10 logarithm of `x`. Return value has the same form as the input. |
| `exp(x: num) -> num` | Computes `e^x`. Return value has the same form as the input. |
| `exp2(x: num) -> num` | Computes `2^x`. Return value has the same form as the input. |
| `exp10(x: num) -> num` | Computes `10^x`. Return value has the same form as the input. |
| `sqrt(x:num) -> num` | Computes the square root of `x`. Return value has the same form as the input. |
| `ceil(x: num) -> num` | Computes the next biggest integer of `x`. Return value has the same form as the input. |
| `floor(x: num) -> num` | Computes the next lowest integer of `x`. Return value has the same form as the input. |
| `round(x: num) -> num` | Computes the closest integer to `x`. Return value has the same form as the input. |
| `fract(x: num) -> num` | Returns the fractional component of `x`. Return value has the same form as the input. |
| `abs(x: num) -> num` | Returns `x` as a positive number. Return value has the same form as the input. |
| `toRad(x: num) -> num` | Converts `x` as degrees into radians. Return value has the same form as the input. |
| `toDeg(x: num) -> num` | Converts `x` as radians into degrees. Return value has the same form as the input. |
| `clamp(x: num, min: num, max: num) -> num` | Clamps `x` so it's always between `min` and `max`. Return value has the same form as `x`. |
| `copysign(mag: num, sign: num) -> num` | Returns `mag` but with the sign of `sign`. Return value has the same form as `mag`. |
| `pan(x: num, pan: num) -> num` | Pans `x` with the [-4.5 dB pan law](http://www.cs.cmu.edu/~music/icm-online/readings/panlaws/index.html#db-pan-law-the-compromise), where a `pan` of -1 is all left and 1 is all right. Return value has the same form as `x`. |
| `left(x: num) -> num` | Returns the left channel of `x` in both channels. Return value has the same form as the input. |
| `right(x: num) -> num` | Returns the right channel of `x` in both channels. Return value has the same form as the input. |
| `swap(x: num) -> num` | Swaps the left and right channels of `x`. Return value has the same form as the input. |
| `combine(left: num, right: num) -> num` | Returns a number with the left channel of `left` and right channel of `right`. Return value has the same form as the input. |
| `mix(a: num, b: num, mix: num) -> num` | Mixes between `a` and `b`, so when `mix` is 0 `a` is returned, and when it's 1 `b` is returned. Return value has the same form as `a`. |
| `sequence(index: num, ...values: num) -> num` | Returns the value at `index` - also works when `index` has different values in both channels. Return value has the same form as the first value. |
| `min(a: num, b: num) -> num` | Returns the minimum of `a` and `b`. Return value has the same form as `a`. |
| `max(a: num, b: num) -> num` | Returns the maximum of `a` and `b`. Return value has the same form as `a`. |

### Filter Functions

| Signature | Description |
| --- | --- |
| `svFilter(in: num, freq: num, q: num) -> (high: num, low: num, band: num, notch: num)` | Applies a state-variable filter to the input. State-variable filters are stable when changing the frequency or Q rapidly, but can destabilize around high frequencies. They also calculate all of the types of outputs at once, which is more efficient than using multiple filters for each. All return values have the same form as `in`. |
| `lowBqFilter(in: num, freq: num, q: num) -> num` | Applies a lowpass biquad filter to the input. Biquad filters are stable at high frequencies but don't like having frequency or Q changed rapidly. Return value has the same form as `in`. |
| `highBqFilter(in: num, freq: num, q: num) -> num` | Applies a highpass biquad filter to the input. Biquad filters are stable at high frequencies but don't like having frequency or Q changed rapidly. Return value has the same form as `in`. |
| `bandBqFilter(in: num, freq: num, q: num) -> num` | Applies a bandpass biquad filter to the input. Biquad filters are stable at high frequencies but don't like having frequency or Q changed rapidly. Return value has the same form as `in`. |
| `notchBqFilter(in: num, freq: num, q: num) -> num` | Applies a notch biquad filter to the input. Biquad filters are stable at high frequencies but don't like having frequency or Q changed rapidly. Return value has the same form as `in`. |
| `allBqFilter(in: num, freq: num, q: num) -> num` | Applies an allpass biquad filter to the input. Biquad filters are stable at high frequencies but don't like having frequency or Q changed rapidly. Return value has the same form as `in`. |
| `peakBqFilter(in: num, freq: num, q: num, gain: num) -> num` | Applies a peak biquad filter to the input. Biquad filters are stable at high frequencies but don't like having frequency or Q changed rapidly. Return value has the same form as `in`. |

### Oscillator Functions

| Signature | Description |
| --- | --- |
| `noise() -> num` | Returns random values between -1 and 1. Return value has the form `[osc]`. |
| `sinOsc(freq: num, phase: num = 0) -> num` | Oscillates between -1 and 1 in a sine wave at the given frequency. Return value has the form `[sin]`. |
| `sqrOsc(freq: num, phase: num = 0, pulseWidth: num = 0.5) -> num` | Oscillates between -1 and 1 in a square wave at the given frequency. Return value has the form `[osc]`. |
| `sawOsc(freq: num, phase: num = 0) -> num` | Oscillates between -1 and 1 in a sawtooth wave at the given frequency. Return value has the form `[osc]`. |
| `triOsc(freq: num, phase: num = 0) -> num` | Oscillates between -1 and 1 in a triangle wave at the given frequency. Return value has the form `[osc]`. |
| `rmpOsc(freq: num, phase: num = 0) -> num` | Oscillates between -1 and 1 in a ramp wave (opposite of a sawtooth) at the given frequency. Return value has the form `[osc]`. |

### MIDI Functions

| Signature | Description |
| --- | --- |
| `note(in: midi) -> (gate: num, note: num, velocity: num, aftertouch:num)` | Gets the current state of the input MIDI stream. `gate`, `velocity`, and `aftertouch` have the form `[none]`. `gate` is 0 when no note is pressed and 1 when a note is pressed, `note` has the form `[note]`. |
| `voices(in: midi, active: num[]) -> midi[]` | Splits the input MIDI stream into multiple so that only one note is playing in each at a time. Pass back in an array of flags for if each index in the array is still making noise, so it's not cut off too early. |
| `channel(in: midi, channel: num) -> midi` | Filters the input MIDI stream so only events occurring on the channel specified are returned. |

### Utility Functions

| Signature | Description |
| --- | --- |
| `adsr(trig: num, a: num, d: num, s: num, r: num) -> (active: num, val: num)` | Returns the active state (1 or 0) and current value of an ADSR curve. |
| `last(x: num) -> num` | Returns the previous sample's value of `x`. |
| `delay(in: num, duration: num) -> num` | Delays `in` by the provided number of seconds. Changing the duration is costly - if you want to change it often, use the three-parameter `delay` overload below. |
| `delay(in: num, amount: num, reserve: num) -> num` | Delays `in` by up to `reserve` seconds. `amount` should be a value between 0 and 1 specifying how much of the buffer to use - change this as much as you want, but avoid changing `reserve`. Form of the return value is form of `in` at the current time - the form isn't delayed. |
| `amplitude(x: num) -> num` | Approximates the amplitude of `x`. Form of the return value is `[amp]`. |
| `hold(in: num, gate: num, else: num = 0) -> num` | When `gate` rises, takes `in` and continues to return it. While `gate` is off returns `else`. Form of the return value is always the form of `in`. |
| `accum(in: num, gate: num, base: num = 0) -> num` | While `gate` is not zero, continuously accumulates `in` and outputs it. When `gate` goes to zero, resets the output to `base.` Form of the return value is always the form of `in`. |
| `mixdown(x: num[]) -> num` | Adds together all active numbers in the input array. Form of the return value is the form of the first entry in the array, whether active or not. |
| `indexed(count: num) -> num[]` | Returns an array with `count` values active, going from 0 to `count - 1`. Can be used similar to a for loop. |

## Forms

The following forms are available:

| [form] | Description  |
| --- | --- |
| `[none]` | Used for numbers that don't have a form, including bare numeric literals. Conversion is always a no-op. |
| `[control]` | The form used to display values in a number control. Internally, controls convert their value to this before displaying in a 0 to 1 range. |
| `[osc]` | An oscillator value, between -1 and 1. |
| `[note]` | A MIDI note value, where 69 is middle A (A4). A numeric literal with a note form can be created by using the syntax `val = :A4` (where `A4` is the note). |
| `[freq]` | A frequency in Hz. A numeric literal with a freq form can be specified by adding "Hz" after the number. |
| `[beats]` | A measure of time in beats. With a BPM of 60, 1 beat = 1 second. A numeric literal can be specified by adding "b" after the number. |
| `[secs]` | A measure of time in seconds. A numeric literal can be specified by adding "s" after the number. |
| `[samples]` | A measure of time in samples. With a sample rate of 44100Hz, 1 sample = 1/44100 seconds. 1 sample is the time step between evaluations of a node. |
| `[db]` | A measure of amplification in decibels. A numeric literal can be specified by adding "dB" after the number. |
| `[amp]` | A linear measure of amplification, used as an intermediary when multiplying a dB value by another number. |
| `[q]` | The resonance of a filter as a Q value. A numeric literal can be specified by adding "Q" after the number. |

## Conversions

The following conversions are available. If you try to perform a conversion that isn't available (e.g. seconds to dB), the result will be the input number with the output form (i.e no conversion will be performed, but the form will change). Also note that conversions are _not_ transitive: converting from `[A] -> [B] -> [C]` is not guaranteed to be the same as converting from `[A] -> [C]`. They _are_ symmetric however, so `[A] -> [B] -> [A]` is the same as performing no conversions.

| From | To | Description |
| --- | --- | --- |
| `[db]` | `[amp]` | Converts decibels to a linear value, useful for multiplication. E.g.: when db = -∞, amp = 0; when db = 0, amp = 1; when db = 6, amp = 2. |
| `[control]` | `[beats]` | Linearly converts from the {0, 1} range to {0, 8}. |
| `[freq]` | `[beats]` | Converts the amount of time of the frequency in Hz to the equivalent number of beats, taking into account the current BPM. |
| `[samples]` | `[beats]` | Converts the number of samples to the equivalent number of beats, taking into account the current sample rate and BPM. |
| `[secs]` | `[beats]` | Converts the number of seconds to the equivalent number of beats, taking into account the current BPM. |
| `[beats]` | `[control]` | Linearly converts from the {0, 8} range to {0, 1}. |
| `[db]` | `[control]` | Remaps the decibel value to a linear value equal to the conversion to `[amp]` divided by 2. |
| `[freq]` | `[control]` | Remaps the frequency value to the {0, 1} range by applying a log curve. |
| `[note]` | `[control]` | Linearly converts from the {0, 127} range to {0, 1}. |
| `[osc]` | `[control]` | Linearly converts from the {-1, 1} range to {0, 1}. |
| `[q]` | `[control]` | Remaps the Q value to the {0, 1} range by applying a curve where a Q of 0.5 is 0, and a Q of 500 is 1. |
| `[samples]` | `[control]` | Remaps the number of samples to a value where 0 samples = 0 and `SAMPLE_RATE` samples = 1. |
| `[secs]` | `[control]` | Remaps the number of seconds to a value where 0 secs = 0 and 5 secs = 1. |
| `[amp]` | `[db]` | Converts a linear value useful for multiplication to decibels. E.g.: when amp = 0, db = -∞; when amp = 1, db = 0; when amp = 2, db = 6. |
| `[control]` | `[db]` | Remaps the linear value to a decibel value equal to the conversion from `[amp]` multiplied by 2. |
| `[beats]` | `[freq]` | Converts the number of beats to the equivalent amount of time in Hz, taking into account the current BPM. |
| `[control]` | `[freq]` | Remaps the {0, 1} range to a frequency value by applying a exponential curve. |
| `[note]` | `[freq]` | Converts the note to its equivalent frequency, where A4 is 440 Hz. |
| `[samples]` | `[freq]` | Converts the number of samples to the equivalent amount of time in Hz, taking into account the current sample rate. |
| `[secs]` | `[freq]` | Converts the number of seconds to the equivalent amount of time in Hz (i.e the reciprocal of the seconds). |
| `[control]` | `[note]` | Linearly converts from the {0, 1} range to {0, 127}. |
| `[freq]` | `[note]` | Converts the frequency into the note that would represent it, where A4 is 440 Hz. |
| `[control]` | `[osc]` | Linearly converts from the {0, 1} range to {-1, 1}. |
| `[control]` | `[q]` | Remaps the control value to a Q by applying the inverse curve of the Q to control conversion. |
| `[beats]` | `[samples]` | Converts the duration of the given beats to the equivalent number of samples, taking into account the current BPM and sample rate. |
| `[control]` | `[samples]` | Remaps the control value to a number of samples where 0 = 0 samples and 1 = `SAMPLE_RATE` samples. |
| `[freq]` | `[samples]` | Converts the amount of time by the given Hz value to the equivalent number of samples, taking into account the current sample rate. |
| `[secs]` | `[samples]` | Converts the duration of the time given to the equivalent number of samples, taking into account the current sample rate. |
| `[beats]` | `[secs]` | Converts the duration of the given beats to the equivalent number of seconds, taking into account the current BPM. |
| `[control]` | `[secs]` | Remaps the control value to a number of seconds where 0 = 0 secs and 1 = 5 secs. |
| `[freq]` | `[secs]` | Converts the amount of time by the given Hz value to the equivalent number of seconds. |
| `[samples]` | `[secs]` | Converts the duration of the given number of samples to the equivalent number of seconds, taking into account the current sample rate. |

## Operators

Operators are used to perform basic operations. The form of the result of an operation is always the form of the left hand operand.

### Math Operators

| Name | Signature | Description |
| --- | --- | --- |
| Add | `a + b` | Adds the two numbers. |
| Subtract | `a - b` | Subtracts `b` from `a`. |
| Multiply | `a * b` | Multiplies the two numbers. |
| Divide | `a / b` | Divides `a` by `b`. |
| Modulo | `a % b` | Calculates the modulo of `a` divided by `b`. |
| Power | `a ^ b` | Raises `a` to the power of `b`. |

### Assignment Operators

| Signature | Description |
| --- | --- |
| `a = b` | Sets the variable `a` to have the value of `b`. |
| `a += b` | Adds `b` to the value of the variable `a`. |
| `a -= b` | Subtracts `b` from the value of the variable `a`. |
| `a *= b` | Sets the variable `a` to be `a` multiplied by `b`. |
| `a /= b` | Sets the variable `a` to be `a` divided by `b`. |
| `a %= b` | Sets the variable `a` to be the modulo of `a` divided by `b`. |
| `a ^= b` | Sets the variable `a` to be `a` to the power of `b`. |

### Unary Operators

| Signature | Description |
| --- | --- |
| `!a` | Returns 1 if `a` is zero, otherwise returns 0. |
| `-a` | Inverts the sign of `a`. |

### Logical Operators

All logical operators use zero for false and any non-zero value for true as the input. The output is either 0 or 1. Logical operators in Maxim are _not_ shortcut, meaning both sides are evaluated to calculate the result.

| Name | Signature | Description |
| --- | --- | --- |
| And | `a && b` | Returns 1 if both `a` and `b` are non-zero, otherwise 0. |
| Or | `a \|\| b` | Returns  1 if either `a` or `b` are non-zero, otherwise 0. |

### Comparison Operators

| Name | Signature | Description |
| --- | --- | --- |
| Equal | `a == b` | Returns 1 if `a` and `b` are equal, otherwise 0. |
| Not Equal | `a != b` | Returns 1 if `a` and `b` are not equal, otherwise 0. |
| Less Than | `a < b` | Returns 1 if `a` is less than `b`, otherwise 0. |
| Greater Than | `a > b` | Returns 1 if `a` is greater than `b`, otherwise 0. |
| Less Than or Equal To | `a <= b` | Returns 1 if `a` is less than or equal to `b`, otherwise 0. |
| Greater Than or Equal To | `a >= b` | Returns 1 if `a` is greater than or equal to `b`, otherwise 0. |

### Bitwise Operators

All bitwise operators convert their parameters to 32-bit integers before performing the operation.

| Name |Signature | Description |
| --- | --- | --- |
| Logical AND | `a & b` | Calculates the bitwise AND of the two values (the bits that are on in both integers). |
| Logical OR | `a \| b` | Calculates the bitwise OR of the two values (the bits that are on in either integer). |
| Logical XOR | `a ^^ b` | Calculates the bitwise XOR of the two values (the bits that are on in one integer, but not the other). |