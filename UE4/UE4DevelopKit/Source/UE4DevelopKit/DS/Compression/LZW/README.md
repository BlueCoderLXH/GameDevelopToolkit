# LZW Compression
## LZW Encode
- Find the longest string __*s*__ in the symbol table that is a prefix of the unscanned input.
- Write the 8-bit value (codeword) associated with __*s*__.
- Scan one character past __*s*__ in the input.
- Associate the next codeword value with __*s*__ + __*c*__ (__*c*__ appended to __*s*__) in the symbol 
table, where __*c*__ is the next character in the input.

## LZW Decode
set the current string __*val*__ to the one-character string consisting of the first character, and perform the following steps
- Write the current string __*val*__.
- Read a codeword __*x*__ from the input.
- Get __*s*__
    - If __*x*__ == __*nextcodeword*__ ( __*x*__ not in symbol table ), set __*val*__ to __*s*__.
    - Else set __*s*__ to the value associated with __*x*__ in the symbol table.
- Associate __*nextcodeword*__ value to __*val*__ + __*c*__ in the symbol table, where __*c*__ is the first character of __*s*__.
- Set the current string __*val*__ to __*s*__.