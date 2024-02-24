# Furox
The Furox Programming Language

## The Furox Grammar* in EBNF (WIP)

&lt;program&gt; ::= &lt;top-level-declaration&gt;*

&lt;top-level-declaration&gt; ::= &lt;include-statement&gt; | &lt;extern-block&gt; | &lt;namespace&gt; | &lt;function-definition&gt; | &lt;struct-definition&gt;

&lt;include-statement&gt; ::= "include" &lt;string-literal&gt; ';'

&lt;extern-block&gt; ::= "extern" '{' (&lt;function-declaration&gt; | &lt;struct-definition&gt;)* '}'

&lt;namespace&gt; ::= "namespace" &lt;identifier&gt; '{' &lt;top-level-declaration&gt;* '}'

&lt;function-signature&gt; ::= &lt;function-specifier&gt;* &lt;identifier&gt; &lt;identifier&gt; &lt;parameter-list&gt;

&lt;parameter-list&gt; ::= '(' [&lt;function-parameter&gt; (',' &lt;function-parameter&gt;)* [',' "..."]] ')'

&lt;function-parameter&gt; ::= &lt;identifier&gt; &lt;identifier&gt;

&lt;function-declaration&gt; ::= &lt;function-signature&gt; ';'

&lt;function-definition&gt; ::= &lt;function-signature&gt; '{' &lt;statement&gt;* '}'

&lt;function-specifier&gt; ::= "export" | "inline"

&lt;struct-definition&gt; ::= &lt;struct-specifier&gt;* "struct" &lt;identifier&gt; '{' &lt;variable-declaration&gt;* '}'

&lt;struct-specifier&gt; ::= "export"

&lt;statement&gt; ::= <!-- TODO: Define this rule properly -->

&lt;string-literal&gt; ::= '"' ((&lt;character&gt; - '"' - '\\') | '\\' ('\\' | ''' | '"' | '?' | 'a' | 'b' | 'f' | 'n' | 'r' | 't' | 'v' | '0'))* '"'

&lt;char-literal&gt; ::= ''' ((&lt;ascii-character&gt; - ''' - '\\') | '\\' ('\\' | ''' | '?' | 'a' | 'b' | 'f' | 'n' | 'r' | 't' | 'v' | '0')) '''

&lt;number&gt; ::= &lt;hex-number&gt; | &lt;octal-number&gt; | &lt;binary-number&gt; | &lt;decimal-number&gt;

&lt;hex-number&gt; ::= "0x" &lt;hex-digit&gt;+

&lt;octal-number&gt; ::= '0' &lt;octal-digit&gt;+

&lt;binary-number&gt; ::= "0b" &lt;binary-digit&gt;+

&lt;decimal-number&gt; ::= &lt;digit&gt;+

&lt;hex-digit&gt; ::= '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' | 'a' | 'b' | 'c' | 'd' | 'e' | 'f' | 'A' | 'B' | 'C' | 'D' | 'E' | 'F'

&lt;octal-digit&gt; ::= '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7'

&lt;binary-digit&gt; ::= '0' | '1'

&lt;digit&gt; ::= '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'

&lt;non-digit&gt; ::= '_' | &lt;lower-case-character&gt; | &lt;upper-case-character&gt;

&lt;lower-case-character&gt; ::= 'a' | 'b' | 'c' | 'd' | 'e' | 'f' | 'g' | 'h' | 'i' | 'j' | 'k' | 'l' | 'm' | 'n' | 'o' | 'p' | 'q' | 'r' | 's' | 't' | 'u' | 'v' | 'w' | 'x' | 'y' | 'z'

&lt;upper-case-character&gt; ::= 'A' | 'B' | 'C' | 'D' | 'E' | 'F' | 'G' | 'H' | 'I | 'J' | 'K' | 'L' | 'M' | 'N' | 'O' | 'P' | 'Q' | 'R' | 'S' | 'T' | 'U' | 'V' | 'W' | 'X' | 'Y' | 'Z'

&lt;ascii-character&gt; ::= any single-byte ASCII character

&lt;character&gt; ::= any Unicode character

*: The bootstrap compiler may not be compliant with the formal grammar specification.
