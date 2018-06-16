use mir::VarType;
use std::fmt;

#[derive(Clone, Copy)]
pub enum FunctionArgRange {
    Precise(usize),      // number of arguments needed is precisely known
    Range(usize, usize), // number of arguments is in a range (optional args)
    VarArg(usize),       // max number of args is unbounded due to a vararg
}

#[derive(Debug, Clone, Copy)]
pub enum Function {
    Cos,
    Sin,
    Log,
    Log2,
    Log10,
    Sqrt,
    Ceil,
    Floor,
    Abs,
    Tan,
    Acos,
    Asin,
    Atan,
    Atan2,
    Hypot,
    ToRad,
    ToDeg,
    Clamp,
    Pan,
    Left,
    Right,
    Swap,
    Combine,
    Mix,
    Sequence,
    Min,
    Max,

    Next,
    Delay,
    Amplitude,
    Hold,
    Accum,
    Mixdown,

    SvFilter,
    LowBqFilter,
    HighBqFilter,
    PeakBqFilter,

    Noise,
    SinOsc,
    SqrOsc,
    SawOsc,
    TriOsc,
    RmpOsc,

    Note,
    Voices,
    Channel,
}

lazy_static! {
    static ref NUM_PARAM: ParamType = ParamType::new(false, VarType::Num);
    static ref MIDI_PARAM: ParamType = ParamType::new(false, VarType::Midi);
    static ref NUM_INTRINSIC_FUNC: FunctionData =
        FunctionData::new(false, VarType::Num, vec![NUM_PARAM.clone()], None);
    static ref TWO_NUM_INTRINSIC_FUNC: FunctionData =
        FunctionData::new(false, VarType::Num, vec![NUM_PARAM.clone(); 2], None);
    static ref CLAMP_DATA: FunctionData =
        FunctionData::new(false, VarType::Num, vec![NUM_PARAM.clone(); 3], None);
    static ref MIX_DATA: FunctionData =
        FunctionData::new(false, VarType::Num, vec![NUM_PARAM.clone(); 3], None);
    static ref SEQUENCE_DATA: FunctionData = FunctionData::new(
        false,
        VarType::Num,
        vec![NUM_PARAM.clone(); 2],
        Some(VarArgType::new(VarType::Num)),
    );
    static ref MINMAX_DATA: FunctionData = FunctionData::new(
        false,
        VarType::Num,
        vec![NUM_PARAM.clone()],
        Some(VarArgType::new(VarType::Num)),
    );
    static ref MIXDOWN_DATA: FunctionData = FunctionData::new(
        false,
        VarType::Num,
        vec![ParamType::new(
            false,
            VarType::new_array(VarType::Num),
        )],
        None,
    );
    static ref CHANNEL_DATA: FunctionData = FunctionData::new(
        false,
        VarType::Midi,
        vec![MIDI_PARAM.clone(), NUM_PARAM.clone()],
        None,
    );
    static ref NEXT_DATA: FunctionData =
        FunctionData::new(true, VarType::Num, vec![NUM_PARAM.clone()], None);
    static ref DELAY_DATA: FunctionData = FunctionData::new(
        true,
        VarType::Num,
        vec![
            NUM_PARAM.clone(),
            NUM_PARAM.clone(),
            ParamType::new(false, VarType::Num),
        ],
        None,
    );
    static ref AMPLITUDE_DATA: FunctionData = FunctionData::new(
        true,
        VarType::Num,
        vec![NUM_PARAM.clone(), ParamType::new(true, VarType::Num)],
        None,
    );
    static ref HOLD_DATA: FunctionData = FunctionData::new(
        true,
        VarType::Num,
        vec![
            NUM_PARAM.clone(),
            NUM_PARAM.clone(),
            ParamType::new(true, VarType::Num),
        ],
        None,
    );
    static ref ACCUM_DATA: FunctionData = FunctionData::new(
        true,
        VarType::Num,
        vec![
            NUM_PARAM.clone(),
            NUM_PARAM.clone(),
            ParamType::new(true, VarType::Num),
        ],
        None,
    );
    static ref SVFILTER_DATA: FunctionData = FunctionData::new(
        true,
        VarType::Tuple(vec![VarType::Num; 4]),
        vec![NUM_PARAM.clone(); 2],
        None,
    );
    static ref BIQUADFILTER_DATA: FunctionData =
        FunctionData::new(true, VarType::Num, vec![NUM_PARAM.clone(); 3], None);
    static ref PEAKBQFILTER_DATA: FunctionData =
        FunctionData::new(true, VarType::Num, vec![NUM_PARAM.clone(); 4], None);
    static ref NOISE_DATA: FunctionData = FunctionData::new(true, VarType::Num, vec![], None);
    static ref PERIODIC_FUNC: FunctionData = FunctionData::new(
        true,
        VarType::Num,
        vec![NUM_PARAM.clone(), ParamType::new(true, VarType::Num)],
        None,
    );
    static ref NOTE_DATA: FunctionData = FunctionData::new(
        true,
        VarType::Tuple(vec![VarType::Num; 4]),
        vec![MIDI_PARAM.clone()],
        None,
    );
    static ref VOICES_DATA: FunctionData = FunctionData::new(
        true,
        VarType::new_array(VarType::Midi),
        vec![
            MIDI_PARAM.clone(),
            ParamType::new(false, VarType::new_array(VarType::Num)),
        ],
        None,
    );
}

#[derive(Debug, Clone)]
pub struct ParamType {
    pub optional: bool,
    pub value_type: VarType,
}

#[derive(Debug, Clone)]
pub struct VarArgType {
    pub value_type: VarType,
}

pub struct FunctionData {
    pub has_side_effects: bool,
    pub return_type: VarType,
    pub arg_types: Vec<ParamType>,
    pub var_arg: Option<VarArgType>,
}

impl fmt::Debug for FunctionArgRange {
    fn fmt(&self, f: &mut fmt::Formatter) -> Result<(), fmt::Error> {
        match self {
            FunctionArgRange::Precise(number) if *number == 1 => write!(f, "1 argument"),
            FunctionArgRange::Precise(number) => write!(f, "{} arguments", number),
            FunctionArgRange::Range(min, max) => write!(f, "between {} and {} arguments", min, max),
            FunctionArgRange::VarArg(number) if *number == 1 => write!(f, "at least 1 argument"),
            FunctionArgRange::VarArg(number) => write!(f, "at least {} arguments", number),
        }
    }
}

impl Function {
    pub fn from_name(name: &str) -> Option<Function> {
        match name {
            "cos" => Some(Function::Cos),
            "sin" => Some(Function::Sin),
            "log" => Some(Function::Log),
            "log2" => Some(Function::Log2),
            "log10" => Some(Function::Log10),
            "sqrt" => Some(Function::Sqrt),
            "ceil" => Some(Function::Ceil),
            "floor" => Some(Function::Floor),
            "abs" => Some(Function::Abs),
            "tan" => Some(Function::Tan),
            "acos" => Some(Function::Acos),
            "asin" => Some(Function::Asin),
            "atan" => Some(Function::Atan),
            "atan2" => Some(Function::Atan2),
            "hypot" => Some(Function::Hypot),
            "toRad" => Some(Function::ToRad),
            "toDeg" => Some(Function::ToDeg),
            "clamp" => Some(Function::Clamp),
            "pan" => Some(Function::Pan),
            "left" => Some(Function::Left),
            "right" => Some(Function::Right),
            "swap" => Some(Function::Swap),
            "combine" => Some(Function::Combine),
            "mix" => Some(Function::Mix),
            "sequence" => Some(Function::Sequence),
            "min" => Some(Function::Min),
            "max" => Some(Function::Max),
            "next" => Some(Function::Next),
            "delay" => Some(Function::Delay),
            "amplitude" => Some(Function::Amplitude),
            "hold" => Some(Function::Hold),
            "accum" => Some(Function::Accum),
            "mixdown" => Some(Function::Mixdown),
            "svFilter" => Some(Function::SvFilter),
            "lowBqFilter" => Some(Function::LowBqFilter),
            "highBqFilter" => Some(Function::HighBqFilter),
            "peakBqFilter" => Some(Function::PeakBqFilter),
            "noise" => Some(Function::Noise),
            "sinOsc" => Some(Function::SinOsc),
            "sqrOsc" => Some(Function::SqrOsc),
            "sawOsc" => Some(Function::SawOsc),
            "triOsc" => Some(Function::TriOsc),
            "rmpOsc" => Some(Function::RmpOsc),
            "note" => Some(Function::Note),
            "voices" => Some(Function::Voices),
            "channel" => Some(Function::Channel),
            _ => None,
        }
    }

    pub fn data(&self) -> &'static FunctionData {
        match self {
            Function::Cos
            | Function::Sin
            | Function::Log
            | Function::Log2
            | Function::Log10
            | Function::Sqrt
            | Function::Ceil
            | Function::Floor
            | Function::Abs
            | Function::Tan
            | Function::Acos
            | Function::Asin
            | Function::Atan
            | Function::ToRad
            | Function::ToDeg
            | Function::Left
            | Function::Right
            | Function::Swap => &NUM_INTRINSIC_FUNC,
            Function::Atan2 | Function::Hypot | Function::Pan | Function::Combine => {
                &TWO_NUM_INTRINSIC_FUNC
            }
            Function::Clamp => &CLAMP_DATA,
            Function::Mix => &MIX_DATA,
            Function::Sequence => &SEQUENCE_DATA,
            Function::Min | Function::Max => &MINMAX_DATA,
            Function::Mixdown => &MIXDOWN_DATA,
            Function::Channel => &CHANNEL_DATA,
            Function::Next => &NEXT_DATA,
            Function::Delay => &DELAY_DATA,
            Function::Amplitude => &AMPLITUDE_DATA,
            Function::Hold => &HOLD_DATA,
            Function::Accum => &ACCUM_DATA,
            Function::SvFilter => &SVFILTER_DATA,
            Function::LowBqFilter | Function::HighBqFilter => &BIQUADFILTER_DATA,
            Function::PeakBqFilter => &PEAKBQFILTER_DATA,
            Function::Noise => &NOISE_DATA,
            Function::SinOsc
            | Function::SqrOsc
            | Function::SawOsc
            | Function::TriOsc
            | Function::RmpOsc => &PERIODIC_FUNC,
            Function::Note => &NOTE_DATA,
            Function::Voices => &VOICES_DATA,
        }
    }

    pub fn has_side_effects(&self) -> bool {
        self.data().has_side_effects
    }

    pub fn return_type(&self) -> &VarType {
        &self.data().return_type
    }

    pub fn arg_types(&self) -> &[ParamType] {
        &self.data().arg_types
    }

    pub fn var_arg(&self) -> &Option<VarArgType> {
        &self.data().var_arg
    }

    pub fn required_args(&self) -> impl Iterator<Item = &ParamType> {
        self.arg_types().iter().filter(|param| !param.optional)
    }

    pub fn arg_range(&self) -> FunctionArgRange {
        let required_count = self.required_args().count();
        if self.var_arg().is_some() {
            FunctionArgRange::VarArg(required_count)
        } else if self.arg_types().len() == required_count {
            FunctionArgRange::Precise(required_count)
        } else {
            FunctionArgRange::Range(required_count, self.arg_types().len())
        }
    }
}

impl ParamType {
    pub fn new(optional: bool, value_type: VarType) -> ParamType {
        ParamType {
            optional,
            value_type,
        }
    }
}

impl VarArgType {
    pub fn new(value_type: VarType) -> VarArgType {
        VarArgType {
            value_type,
        }
    }
}

impl FunctionData {
    pub fn new(
        has_side_effects: bool,
        return_type: VarType,
        arg_types: Vec<ParamType>,
        var_arg: Option<VarArgType>,
    ) -> FunctionData {
        FunctionData {
            has_side_effects,
            return_type,
            arg_types,
            var_arg,
        }
    }
}
