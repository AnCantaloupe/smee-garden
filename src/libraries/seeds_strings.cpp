#define STRING(String)              (string{ (String), StringLength((string{ String, 4096 })) })
#define STRING_FROM_LITERAL(String) (string{ (char *)(String), sizeof((String)) - 1 })
#define STRING_FROM_ARRAY(String)   (string{ (char *)(String), sizeof((String)) })

internal s64
StringLength(string String) {
    
    s64 Result = 0;
    while (String.Length-- && *String.Data++) {
        Result++;
    }
    
    return Result;
}

internal b32
StringsIdentical(string StringA, string StringB) {
    
    b32 Result;
    
    s64 Difference = StringA.Length - StringB.Length;
    while (!Difference && StringA.Length-- && StringB.Length--) {
        Difference = *StringA.Data++ - *StringB.Data++;
    }
    
    Result = !Difference;
    return Result;
}

internal b32
StringStartsWith(string String, string Comparee) {
    
    b32 Result;
    
    size_t Difference = 0;
    while (String.Length-- && Comparee.Length--) {
        Difference = *String.Data++ - *Comparee.Data++;
        if (Difference) break;
    }
    
    Result = !Difference;
    return Result;
}

internal b32
CharEq(char Char, string Comp) {
    
    for (int I = 0; I < Comp.Length; I++) if (Char == Comp.Data[I]) return 1;
    return 0;
}

internal b32
StringContains(string String, string Comp) {
    
    for (int I = 0; I < String.Length; I++) {
        for (int J = 0; J < Comp.Length; J++) {
            if (String.Data[I] == Comp.Data[J]) return 1;
        }
    }
    return 0;
}

internal b32
StringContainsOther(string String, string Comp) {
    
    for (int I = 0; I < String.Length; I++) {
        for (int J = 0; J < Comp.Length; J++) {
            if (String.Data[I] == Comp.Data[J]) break;
            if (J == Comp.Length - 1) return 1;
        }
    }
    return 0;
}

#include <stdarg.h>
// @Cleanup(canta): Prob should fix Dest.Length
internal string
CatStrings(int NoStrings, string Dest, ...) {
    
    va_list Args;
    va_start(Args, Dest);
    for (int I = 0; I < NoStrings; I++) {
        
        string Source = va_arg(Args, string);
        
        while (Source.Length-- > 0) {
            if (!(Dest.Length-- > 0)) goto loopbreak;
            *Dest.Data++ = *Source.Data++;
        }
    }
    loopbreak:
    
    va_end(Args);
    
    return Dest;
}

inline string
CopyString(string Dest, string Source) {
    
    return CatStrings(1, Dest, Source);
}

inline string
CatStringsNull(int NoStrings, string Dest, ...) {
    
    va_list Args;
    va_start(Args, Dest);
    for (int I = 0; I < NoStrings; I++) {
        
        string Source = va_arg(Args, string);
        
        while (Source.Length--) {
            if (!(--Dest.Length)) goto loopbreak;
            *Dest.Data++ = *Source.Data++;
        }
    }
    loopbreak:
    *Dest.Data = '\0';
    
    va_end(Args);
    
    return Dest;
}

inline string
CopyStringNull(string Dest, string Source) {
    
    return CatStringsNull(1, Dest, Source);
}

inline string
AdvanceString(string *String, s64 Count) {
    
    string Result = *String;
    
    if (Count <= String->Length) {
        String->Data += Count;
        String->Length -= Count;
    }
    else {
        String->Data += String->Length;
        String->Length = 0;
    }
    
    return Result;
}

inline void
WriteChar(string *String, char Character) {
    
    if (String->Length) {
        *String->Data++ = Character;
        String->Length--;
    }
}

internal s64
EndOfToken(string String, string Delimiter = STRING_FROM_LITERAL(" \t\r\n\0")) {
    
    int Index = 0;
    while (String.Length-- && !CharEq(String.Data[Index], Delimiter)) {
        
        Index++;
    }
    
    return Index;
}

internal string
EatToNextToken(string *String, string Delimiter = STRING_FROM_LITERAL(" \t\r\n\0")) {
    
    string TummyContents = { String->Data, 0 };
    
    if (String->Length) {
        TummyContents.Length = EndOfToken(*String, Delimiter);
        Assert(TummyContents.Length <= String->Length);
        
        String->Data   += TummyContents.Length;
        String->Length -= TummyContents.Length;
        
        while (String->Length > 0 && CharEq(String->Data[0], Delimiter)) {
            String->Data++;
            String->Length--;
        }
        
    }
    
    return TummyContents;
}

internal string
EatToNextLine(string *String) {
    
    string TummyContents = { String->Data, 0 };
    
    while (String->Length > 0) {
        if (   (String->Data[0] == '\r') && (String->Data[1] == '\n')
            || (String->Data[0] == '\n') && (String->Data[1] == '\r')) {
            String->Data   += 2;
            String->Length -= 2;
            break;
        }
        else if ((String->Data[0] == '\r') || (String->Data[0] == '\n') || (String->Data[0] == '\0'))  {
            String->Data   += 1;
            String->Length -= 1;
            break;
        }
        else {
            String->Data   += 1;
            String->Length -= 1;
            TummyContents.Length++;
        }
    }
    
    return TummyContents;
}

internal string
GetFilenameFromPath(string Path) {
    
    string Filename = Path;
    while (Path.Length--) {
        char Char = *Path.Data++;
        if (Char == '\\' || Char == '/') Filename = Path;
    }
    return Filename;
}

internal void
StripSurroundingWhitespace(string *String, string Whitespace = STRING_FROM_LITERAL(" \t\r\n\0")) {
    
    if (String->Length > 0) {
        while(String->Length > 0) {
            if (CharEq(*String->Data, Whitespace)) {
                String->Data++;
                String->Length--;
            }
            else {
                break;
            }
        }
        
        int LastActualCharacter = 0;
        for (int I = 0; I < String->Length; I++) {
            if (!CharEq(String->Data[I], Whitespace)) LastActualCharacter = I;
        }
        String->Length = LastActualCharacter + 1;
    }
}

internal int
CountLines(string String) {
    
    int Result = 0;
    
    char *Ptr = String.Data;
    while (String.Length > 0) {
        EatToNextLine(&String);
        Result++;
    }
    
    return Result;
}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

#define HEX_32_STRING_LENGTH (2 + 8)
#define HEX_64_STRING_LENGTH (2 + 16)

// TODO(canta): Kinda goofy atm, track down that siq thread on r/C_Programming
internal int
CountDigits(u32 Value) {
    
    int Result = 1;
    while (Value /= 10) Result++;
    return Result;
}

// To DEC
internal string
U32ToDecString(string String, u32 Value) {
    
    int DigitsReq = CountDigits(Value);
    int Mask = (int)Power((s64)10, DigitsReq - 1);
    int FirstNonZero = -1;
    for (int Digit = 0; Digit < DigitsReq; Digit++) {
        char DigitValue = (char)(Value / Mask);
        char Char = DigitValue + '0';
        
        if (Char == '0') {
            FirstNonZero = Digit;
        }
        
        Value -= DigitValue * Mask;
        Mask /= 10;
        
        WriteChar(&String, Char);
    }
    
    return String;
}

internal string
S32ToDecString(string String, s32 Value) {
    
    if (Value < 0) {
        WriteChar(&String, '-');
        Value = -Value;
    }
    
    return U32ToDecString(String, Value);
}

// TODO(canta): This is not very precise at the moment!
internal string
F32ToDecString(string String, f32 Value, b32 TrailingEff = 0, int DecimalPlaces = 4) {
    
    Assert(DecimalPlaces >= 0);
    
    int Index = 0;
    if (Value < 0) {
        WriteChar(&String, '-');
        Value = -Value;
    }
    
    f32 ValueCopy = Value;
    
    f32 Mask = 1.0f;
    int Counter = 0;
    if (Value < Mask) {
        WriteChar(&String, '0');
    }
    else {
        while (Mask <= Value) {
            Mask *= 10.0f;
            Counter++;
        }
    }
    
    Mask /= 10.0f;
    while (Counter--) {
        char Digit = (char)(Value / Mask);
        char Char = Digit + '0';
        
        WriteChar(&String, Char);
        
        Value -= Digit * Mask;
        Mask /= 10.0f;
    }
    
    if (DecimalPlaces != 0) {
        WriteChar(&String, '.');
        
        Value = ValueCopy;
        Value -= (f32)((u32)Value);
        while (DecimalPlaces--) {
            Value *= 10.0f;
            char Digit = (char)Value;
            char Char = Digit + '0';
            
            WriteChar(&String, Char);
            
            Value -= Digit;
        }
    }
    
    if (TrailingEff) WriteChar(&String, 'f');
    
    return String;
}

// From DEC
internal s32
S32FromDecString(string String) {
    
    s32 Result = 0;
    
    char *Ptr = String.Data;
    
    b32 Negative = 0;
    if (*Ptr == '-') {
        Ptr++;
        String.Length--;
        Negative = 1;
    }
    
    while (String.Length--) {
        Result *= 10;
        Result += *Ptr++ - '0';
    }
    
    return Negative ? -Result : Result;
}

internal u32
U32FromDecString(string String) {
    
    u32 Result = 0;
    
    while (String.Length--) {
        Result *= 10;
        Result += *String.Data++ - '0';
    }
    
    return Result;
}

// TODO(canta): I kind of hate this. I'd like to do a proper conversion eventually, since its kind of hard to keep track of all the indices (which are all totally arbitrary at the moment and are likely to change).
// @Robustness
internal f32
F32FromDecString(string String) {
    
    f32 Result = 0.0f;
    
    b32 Negative = 0;
    if (String.Data[0] == '-') {
        Negative = 1;
        String.Data++;
        String.Length--;
    }
    
    string PreviousToken;
    PreviousToken = EatToNextToken(&String, STRING_FROM_LITERAL(". \t\r\n\0"));
    u32 Whole     = U32FromDecString(PreviousToken);
    PreviousToken = EatToNextToken(&String, STRING_FROM_LITERAL("f \t\r\n\0"));
    u32 Fraction  = U32FromDecString(PreviousToken);
    f32 FracMul   = Power(0.1f, PreviousToken.Length);
    
    Result += (f32)Whole + (f32)Fraction * FracMul;
    
    return Negative ? -Result : Result;
}

// To HEX
internal string
U32ToHexString(string String, u32 Value) {
    
    Assert(String.Length >= HEX_32_STRING_LENGTH);
    
    WriteChar(&String, '0');
    WriteChar(&String, 'x');
    
    int *ValueAddress = (int *)&Value;
    for (int ChunkIndex = 7; ChunkIndex >= 0; ChunkIndex--) {
        char ChunkValue = ((*ValueAddress >> (ChunkIndex * 4)) & 0xF);
        char Char  = ChunkValue < 0xA ? ChunkValue + '0' : ChunkValue - 10 + 'A';
        
        WriteChar(&String, Char);
    }
    
    return String;
}

internal string
F32ToHexString(string String, f32 Value) {
    
    Assert(String.Length >= HEX_32_STRING_LENGTH);
    
    WriteChar(&String, '0');
    WriteChar(&String, 'x');
    
    int *ValueAddress = (int *)&Value;
    for (int ChunkIndex = 7; ChunkIndex >= 0; ChunkIndex--) {
        char ChunkValue = ((*ValueAddress >> (ChunkIndex * 4)) & 0xF);
        char Char = ChunkValue < 0xA ? ChunkValue + '0' : ChunkValue - 10 + 'A';
        
        WriteChar(&String, Char);
    }
    
    return String;
}

// From HEX
internal u32
U32FromHexString(string String) {
    
    Assert(String.Length >= HEX_32_STRING_LENGTH);
    
    u32  Result = 0;
    
    for (int CharIndex = 2; CharIndex < 10; CharIndex++) {
        char Character = String.Data[CharIndex];
        char ChunkValue = Character <= '9' ? Character - '0' : Character + 10 - 'A';
        
        Result |= ChunkValue << ((7 - CharIndex + 2) * 4);
    }
    
    return Result;
}

internal f32
F32FromHexString(string String) {
    
    Assert(String.Length >= HEX_32_STRING_LENGTH);
    
    f32  Result = 0.0f;
    int *ResultAddress = (int *)&Result;
    
    for (int CharIndex = 2; CharIndex < 10; CharIndex++) {
        char Character = String.Data[CharIndex];
        char ChunkValue = Character <= '9' ? Character - '0' : Character + 10 - 'A';
        
        *ResultAddress |= (ChunkValue << ((7 - CharIndex + 2) * 4));
    }
    
    return Result;
}

// Usage: d : a/#
internal b32
ParseF32DecHexPair(string String, f32 *Dec, f32 *Hex) {
    
    b32 Result;
    
    *Dec = F32FromDecString(EatToNextToken(&String));
    
    EatToNextToken(&String);
    
    Result = (String.Data[0] != '#');
    if (Result) *Hex = F32FromHexString(EatToNextToken(&String));
    
    return Result;
}

// Usage: d d : a/# a/#
internal v2b
ParseV2DecHexPair(string String, v2 *Dec, v2 *Hex) {
    
    v2b Result;
    
    Dec->x = F32FromDecString(EatToNextToken(&String));
    Dec->y = F32FromDecString(EatToNextToken(&String));
    
    EatToNextToken(&String);
    
    Result.x = String.Data[0] != '#';
    if (Result.x) Hex->x = F32FromHexString(EatToNextToken(&String));
    else          EatToNextToken(&String);
    Result.y = String.Data[0] != '#';
    if (Result.y) Hex->y = F32FromHexString(EatToNextToken(&String));
    else          EatToNextToken(&String);
    
    return Result;
}

// Usage: d d d : a/# a/# a/#
internal v3b
ParseV3DecHexPair(string String, v3 *Dec, v3 *Hex) {
    
    v3b Result;
    
    Dec->x = F32FromDecString(EatToNextToken(&String));
    Dec->y = F32FromDecString(EatToNextToken(&String));
    Dec->z = F32FromDecString(EatToNextToken(&String));
    
    EatToNextToken(&String);
    
    Result.x = String.Data[0] != '#';
    if (Result.x) Hex->x = F32FromHexString(EatToNextToken(&String));
    else          EatToNextToken(&String);
    Result.y = String.Data[0] != '#';
    if (Result.y) Hex->y = F32FromHexString(EatToNextToken(&String));
    else          EatToNextToken(&String);
    Result.z = String.Data[0] != '#';
    if (Result.z) Hex->z = F32FromHexString(EatToNextToken(&String));
    else          EatToNextToken(&String);
    
    return Result;
}

internal string
Print(string Dest, string Format, ...) {
    
    va_list Args;
    va_start(Args, Format);
    for (int I = 0; I < Format.Length; I++) {
        switch (Format.Data[I]) {
            case '%': {
                I++;
                switch (Format.Data[I]) {
                    case 'c': { char Source = va_arg(Args, char);      WriteChar(&Dest, Source); }            break;
                    case 's': { string Source = { va_arg(Args, char *), va_arg(Args, s64) };  Dest = CopyString(Dest, Source); }     break; // TODO(canta): Change instances of this in wsol
                    case 'd': { s32 Source = va_arg(Args, s32);        Dest = S32ToDecString(Dest, Source); } break;
                    case 'u': { u32 Source = va_arg(Args, u32);        Dest = U32ToDecString(Dest, Source); } break;
                    case 'f': { f32 Source = (f32)(va_arg(Args, f64)); Dest = F32ToDecString(Dest, Source); } break;
                    case 'x': { u32 Source = va_arg(Args, u32);        Dest = U32ToHexString(Dest, Source); } break;
                    case 'a': { f32 Source = (f32)(va_arg(Args, f64)); Dest = F32ToHexString(Dest, Source); } break;
                    default:  { Assert(!"Invalid/not yet implemented!"); }                                    break;
                };
            } break;
            default: { WriteChar(&Dest, Format.Data[I]); } break;
        };
    }
    va_end(Args);
    
    return Dest;
}



// TODO(canta): BAD NAME QUARANTINE
inline v3
ReadV3Dec(string *Cursor) {
    
    v3 Result;
    
    string Line = EatToNextLine(Cursor);
    EatToNextToken(&Line);
    Result.x = F32FromDecString(EatToNextToken(&Line));
    Result.y = F32FromDecString(EatToNextToken(&Line));
    Result.z = F32FromDecString(EatToNextToken(&Line));
    
    return Result;
}

inline v3
Read3AnglesDeg(string *Cursor) {
    
    v3 Result = ReadV3Dec(Cursor);
    Result.x = DegToRad(Result.x);
    Result.y = DegToRad(Result.y);
    Result.z = DegToRad(Result.z);
    return Result;
}

inline s32
ReadS32Dec(string *Cursor) {
    
    s32 Result;
    
    string Line = EatToNextLine(Cursor);
    EatToNextToken(&Line);
    Result = S32FromDecString(EatToNextToken(&Line));
    
    return Result;
}

inline f32
ReadF32Dec(string *Cursor) {
    
    f32 Result;
    
    string Line = EatToNextLine(Cursor);
    EatToNextToken(&Line);
    Result = F32FromDecString(EatToNextToken(&Line));
    
    return Result;
}

inline u32
ReadU32Hex(string *Cursor) {
    
    u32 Result;
    
    string Line = EatToNextLine(Cursor);
    EatToNextToken(&Line);
    Result = U32FromHexString(EatToNextToken(&Line));
    
    return Result;
}

inline void
Read2U32Hex(string *Cursor, u32 *DestA, u32 *DestB) {
    
    string Line = EatToNextLine(Cursor);
    EatToNextToken(&Line);
    *DestA = U32FromHexString(EatToNextToken(&Line));
    *DestB = U32FromHexString(EatToNextToken(&Line));
}

