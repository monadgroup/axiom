use crate::mir::VarType;
use std::fmt;

#[derive(Clone, Copy, Debug)]
pub enum FunctionArgRange {
    Precise(usize),      // number of arguments needed is precisely known
    Range(usize, usize), // number of arguments is in a range (optional args)
    VarArg(usize),       // max number of args is unbounded due to a vararg
}

macro_rules! func {
    (($($arg_type:expr),*) -> $return_type:expr) => (
        FunctionData::new($return_type, vec![$( ParamType::new(false, $arg_type), )*], None)
    );
    (($($arg_type:expr),*, $(?$optional_arg_type:expr),*) -> $return_type:expr) => (
        FunctionData::new($return_type, vec![$( ParamType::new(false, $arg_type), )* $( ParamType::new(true, $optional_arg_type), )*], None)
    );
    (($($arg_type:expr),* => $vararg_type:expr) -> $return_type:expr) => (
        FunctionData::new($return_type, vec![$( ParamType::new(false, $arg_type), )*], Some($vararg_type))
    );
}

macro_rules! define_functions {
    ($($enum_name:ident = $str_name:tt $data:expr ),*) => (
        #[derive(Debug, Clone, Copy)]
        pub enum Function {
            $( $enum_name, )*
        }

        impl Function {
            pub fn from_name(name: &str) -> Option<Function> {
                match name {
                    $( $str_name => Some(Function::$enum_name), )*
                    _ => None
                }
            }

            pub fn data(&self) -> FunctionData {
                use crate::mir::VarType::*;
                match self {
                    $( Function::$enum_name => $data, )*
                }
            }
        }

        impl fmt::Display for Function {
            fn fmt(&self, f: &mut fmt::Formatter) -> Result<(), fmt::Error> {
                match self {
                    $( Function::$enum_name => write!(f, $str_name), )*
                }
            }
        }

        pub const FUNCTION_TABLE: [&str; 59] = [$($str_name, )*];
    );
}

define_functions! {
    Sin = "sin" func![(Num) -> Num],
    Cos = "cos" func![(Num) -> Num],
    Tan = "tan" func![(Num) -> Num],
    Min = "min" func![(Num, Num) -> Num],
    Max = "max" func![(Num, Num) -> Num],
    Sqrt = "sqrt" func![(Num) -> Num],
    Floor = "floor" func![(Num) -> Num],
    Ceil = "ceil" func![(Num) -> Num],
    Round = "round" func![(Num) -> Num],
    Abs = "abs" func![(Num) -> Num],
    CopySign = "copysign" func![(Num, Num) -> Num],
    Fract = "fract" func![(Num) -> Num],
    Exp = "exp" func![(Num) -> Num],
    Exp2 = "exp2" func![(Num) -> Num],
    Exp10 = "exp10" func![(Num) -> Num],
    Log = "log" func![(Num) -> Num],
    Log2 = "log2" func![(Num) -> Num],
    Log10 = "log10" func![(Num) -> Num],
    Asin = "asin" func![(Num) -> Num],
    Acos = "acos" func![(Num) -> Num],
    Atan = "atan" func![(Num) -> Num],
    Atan2 = "atan2" func![(Num, Num) -> Num],
    Sinh = "sinh" func![(Num) -> Num],
    Cosh = "cosh" func![(Num) -> Num],
    Tanh = "tanh" func![(Num) -> Num],
    Hypot = "hypot" func![(Num, Num) -> Num],
    ToRad = "toRad" func![(Num) -> Num],
    ToDeg = "toDeg" func![(Num) -> Num],
    Clamp = "clamp" func![(Num, Num, Num) -> Num],
    Pan = "pan" func![(Num, Num) -> Num],
    Left = "left" func![(Num) -> Num],
    Right = "right" func![(Num) -> Num],
    Swap = "swap" func![(Num) -> Num],
    Combine = "combine" func![(Num, Num) -> Num],
    Mix = "mix" func![(Num, Num, Num) -> Num],
    Sequence = "sequence" func![(Num => Num) -> Num],
    Next = "next" func![(Num) -> Num],
    Delay = "delay" func![(Num, Num, ?Num) -> Num],
    Amplitude = "amplitude" func![(Num) -> Num],
    Hold = "hold" func![(Num, Num, ?Num) -> Num],
    Accum = "accum" func![(Num, Num, ?Num) -> Num],
    Mixdown = "mixdown" func![(VarType::new_array(Num)) -> Num],
    SvFilter = "svFilter" func![(Num, Num, Num) -> Tuple(vec![Num, Num, Num, Num])],
    LowBqFilter = "lowBqFilter" func![(Num, Num, Num) -> Num],
    HighBqFilter = "highBqFilter" func![(Num, Num, Num) -> Num],
    BandBqFilter = "bandBqFilter" func![(Num, Num, Num) -> Num],
    NotchBqFilter = "notchBqFilter" func![(Num, Num, Num) -> Num],
    AllBqFilter = "allBqFilter" func![(Num, Num, Num) -> Num],
    PeakBqFilter = "peakBqFilter" func![(Num, Num, Num, Num) -> Num],
    Noise = "noise" func![() -> Num],
    SinOsc = "sinOsc" func![(Num, ?Num) -> Num],
    SqrOsc = "sqrOsc" func![(Num, ?Num, ?Num) -> Num],
    SawOsc = "sawOsc" func![(Num, ?Num) -> Num],
    TriOsc = "triOsc" func![(Num, ?Num) -> Num],
    RmpOsc = "rmpOsc" func![(Num, ?Num) -> Num],
    Note = "note" func![(Midi) -> Tuple(vec![Num, Num, Num, Num])],
    Voices = "voices" func![(Midi, VarType::new_array(Num)) -> VarType::new_array(Midi)],
    Channel = "channel" func![(Midi, Num) -> Midi],
    Indexed = "indexed" func![(Num) -> VarType::new_array(Num)]
}

#[derive(Debug, Clone)]
pub struct ParamType {
    pub optional: bool,
    pub value_type: VarType,
}

pub struct FunctionData {
    pub return_type: VarType,
    pub arg_types: Vec<ParamType>,
    pub var_arg: Option<VarType>,
}

impl fmt::Display for FunctionArgRange {
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
    pub fn return_type(&self) -> VarType {
        self.data().return_type
    }

    pub fn arg_types(&self) -> Vec<ParamType> {
        self.data().arg_types
    }

    pub fn var_arg(&self) -> Option<VarType> {
        self.data().var_arg
    }

    pub fn required_args(&self) -> Vec<ParamType> {
        self.arg_types()
            .into_iter()
            .filter(|param| !param.optional)
            .collect()
    }

    pub fn arg_range(&self) -> FunctionArgRange {
        let required_count = self
            .arg_types()
            .into_iter()
            .filter(|param| !param.optional)
            .count();
        if self.var_arg().is_some() {
            FunctionArgRange::VarArg(required_count + 1)
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

impl FunctionData {
    pub fn new(
        return_type: VarType,
        arg_types: Vec<ParamType>,
        var_arg: Option<VarType>,
    ) -> FunctionData {
        FunctionData {
            return_type,
            arg_types,
            var_arg,
        }
    }
}
