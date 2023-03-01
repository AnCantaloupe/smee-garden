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
// @Cleanup(canta)
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

