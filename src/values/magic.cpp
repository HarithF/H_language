#define H_KEY(m) \
m(K_auto, "auto") \
m(K_break, "break") \
m(K_case, "case") \
m(K_char,"char")\
m(K_const,"const")\
m(K_continue,"continue")\
m(K_default,"default")\
m(K_do,"do")\
m(K_double,"double")\
m(K_else,"else")\
m(K_enum,"emi,")\
m(K_extern,"extern")\
m(K_float,"float")\
m(K_for,"for")\
m(K_goto,"goto")\
m(K_if,"if")\
m(K_inline,"inline")\
m(K_int,"int")\
m(K_long,"long")\
m(K_register,"regoster")\
m(K_restrict,"restrict")\
m(K_return,"return")\
m(K_short,"short")\
m(K_signed,"signed")\
m(K_sizeof,"sizeof") \
m(K_static,"static")\
m(K_struct,"struct")\
m(K_switch,"switch")\
m(K_typedef,"typedef")\
m(K_union,"union")\
m(K_unsigned,"unsigned")\
m(K_void,"void")\
m(K_volatile,"volatile")\
m(K_while,"while")\
m(K_Alignas,"_Alignas")\
m(K_Alignof,"_Alignof")\
m(K_Atomic,"_Atomic")\
m(K_Bool,"_Bool")\
m(K_Complex,"_Complex")\
m(K_Generic,"_Generic")\
m(K_Imaginary,"_Imaginary")\
m(K_Noreturn,"_Noreturn")\
m(K_Static_assert,"_Static_assert")\
m(K_Thread_local,"_Thread_local")\


//m(Token, written, actual, higher) //left-to-right
//m(Token, written, actual, lower) //right-to-left
#define H_OP(m) \
/* Multiplicative (L2R) */\
m(P_Multiplication,     "*",    Multiplicative, Unary) \
m(P_Division,           "/",    Multiplicative, Unary) \
m(P_Modulo,             "%",    Multiplicative, Unary) \
/* Additive (L2R) */\
m(P_Addition,           "+",    Additive, Multiplicative) \
m(P_Substraction,       "-",    Additive, Multiplicative) \
/* Shift (L2R) */\
m(P_Bitwise_Shift_L,    "<<",   Shift, Additive) \
m(P_Bitwise_Shift_R,    ">>",   Shift, Additive) \
/* Relational (L2R) */ \
m(P_Less,               "<",    Relational, Shift) \
m(P_Greater,            ">",    Relational, Shift) \
m(P_Less_Equal,         "<=",   Relational, Shift) \
m(P_Greater_Equal,      ">=",   Relational, Shift) \
/* Equality (L2R) */ \
m(P_Equal,              "==",   Equality, Relational) \
m(P_Unequal,            "!=",   Equality, Relational) \
/* Logic Stuff (L2R) */ \
m(P_Bitwise_And,        "&",    BitwiseAND, Equality) \
m(P_Bitwise_Xor,        "^",    BitwiseXOR, BitwiseAND) \
m(P_Bitwise_Or,         "|",    BitwiseOR, BitwiseXOR) \
m(P_Logical_And,        "&&",   LogicalAND, BitwiseOR) \
m(P_Logical_Or,         "||",   LogicalOR, LogicalAND) \
/* Conditional (R2L) */ \
m(P_Inline_If,          "?",    Conditional, Assignment) \
/* Assignment (R2L) */ \
m(P_Assign,                 "=",     Assignment, Comma) \
m(P_Addition_Assign,        "+=",    Assignment, Comma) \
m(P_Substraction_Assign,    "-=",    Assignment, Comma) \
m(P_Multiplication_Assign,  "*=",    Assignment, Comma) \
m(P_Division_Assign,        "/=",    Assignment, Comma) \
m(P_Modulo_Assign,          "%=",    Assignment, Comma) \
m(P_Bitwise_And_Assign,     "&=",    Assignment, Comma) \
m(P_Bitwise_Or_Assign,      "|=",    Assignment, Comma) \
m(P_Bitwise_Xor_Assign,     "^=",    Assignment, Comma) \
m(P_Bitwise_Shift_L_Assign, "<<=",   Assignment, Comma) \
m(P_Bitwise_Shift_R_Assign, ">>=",   Assignment, Comma) \


#define H_LIT(m) \
m(C_Integer,        "<integer constant>") \
m(C_Character,      "<character constant>") \
m(S_Literal,        "<string literal>")

#define H_TOK(m) \
/* Misc */ \
m(M_EoF,            "<eof>") \
m(M_Id,             "<identifier>") \
/* Delimiters */ \
m(D_Brace_L,        "{") \
m(D_Brace_R,        "}") \
m(D_Parenthesis_L,  "(") \
m(D_Parenthesis_R,  ")") \
m(D_Bracket_L,      "[") \
m(D_Bracket_R,      "]") \
/* Other punctuators (Comma, Postfix) */ \
m(P_Dot,            ".") \
m(P_Tripple_Dot,    "...") \
m(P_Arrow_R,        "->") \
m(P_Colon,          ":") \
m(P_Semicolon,      ";") \
m(P_Comma,          ",") \
m(P_Preprocessor_Stringize,     "#") \
m(P_Preprocessor_Concat,        "##") \
m(P_Increment,      "++") \
m(P_Decrement,      "--") \
m(P_Logical_Not,    "!") \
m(P_Bitwise_Not,    "~")
