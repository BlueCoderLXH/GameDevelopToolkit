using System.Collections.Generic;
using System;

///<summary>
/// FastString
/// 
/// Mutable String class, optimized for speed and memory allocations while retrieving the final result as a string.
/// Similar use than StringBuilder, but avoid a lot of allocations done by StringBuilder (conversion of int and float to string, frequent capacity change, etc.)
/// 
/// PS: Don't new 'FastString' object yourself, call FastString.Create instead
///</summary>
public partial class FastString
{
    private const int DefaultCapacity = 128;


    /// <summary>
    /// Cache the string object
    /// </summary>
    private string m_generatedString;
    ///<summary>
    /// Is m_stringGenerated is up to date ?
    ///</summary>
    private bool m_isStringGenerated;

    ///<summary>
    /// Working mutable string
    ///</summary>
    private char[] m_buffer;
    private int m_bufferPos;
    private int m_charsCapacity;

    ///<summary>
    /// Temporary string used for the Replace method
    ///</summary>
    private List<char> m_replacement;

    /// <summary>
    /// Is string empty
    /// </summary>
    /// <returns></returns>
    public bool IsEmpty => m_bufferPos == 0;

    /// <summary>
    /// Create a new FastString with a specified capacity
    /// </summary>
    internal FastString(int initialCapacity = DefaultCapacity)
    {
        m_buffer = new char[m_charsCapacity = initialCapacity];
        m_bufferPos = 0;

        m_generatedString = "";
        m_isStringGenerated = false;

        m_replacement = new List<char>(m_charsCapacity);

        m_nextNode = null;
    }

    /// <summary>
    /// Create a new FastString with a string object
    /// </summary>
    internal FastString(string str_value) : this()
    {
        Set(str_value);
    }

    ///<summary>
    /// Set a string, no memorry allocation
    ///</summary>
    public void Set(string str)
    {
        // We fill the m_chars list to manage future appends, but we also directly set the final stringGenerated
        Clear();

        Append(str);

        m_generatedString = str;
        m_isStringGenerated = true;
    }

    ///<summary>
    /// Append a string without memory allocation
    ///</summary>
    public FastString Append(string value)
    {
        ChecklExpandBuffer(value.Length);

        int n = value.Length;
        for (int i = 0; i < n; i++)
            m_buffer[m_bufferPos + i] = value[i];

        m_bufferPos += n;
        m_isStringGenerated = false;

        return this;
    }

    ///<summary>
    /// Append an object.ToString(), allocate some memory
    ///</summary>
    public FastString Append(object value)
    {
        Append(value.ToString());
        return this;
    }

    /// <summary>
    /// Append a boolean value
    /// </summary>
    public FastString Append(bool flag)
    {
        Append(flag ? "true" : "false");
        return this;
    }

    ///<summary>
    /// Append an integer value
    ///</summary>
    public FastString Append(byte value)
    {
        return Append_Integer(value);
    }
    public FastString Append(short value)
    {
        return Append_Integer(value);
    }
    public FastString Append(int value)
    {
        return Append_Integer(value);
    }
    public FastString Append(long value)
    {
        return Append_Integer(value);
    }

    ///<summary>
    /// Append a float value
    ///</summary>
    public FastString Append(float valueF)
    {
        return Append_Float(valueF);
    }
    public FastString Append(double valueF)
    {
        return Append_Float(valueF);
    }

    ///<summary>
    /// Replace all occurences of a string by another one
    ///</summary>
    public FastString Replace(string oldStr, string newStr)
    {
        return ReplaceInternal(oldStr, newStr, newStr.Length);
    }
    public FastString Replace(string oldStr, FastString newStr)
    {
        return ReplaceInternal(oldStr, newStr.m_buffer, newStr.m_bufferPos);
    }

    ///<summary>
    /// Reset the m_char array
    ///</summary>
    public FastString Clear()
    {
        m_bufferPos = 0;
        m_isStringGenerated = false;
        m_nextNode = null;
        return this;
    }

    #region Internal Algorithm
    ///<summary>
    /// Append an integer without memory allocation
    ///</summary>
    private FastString Append_Integer(long integer_value)
    {
        // Allocate enough memory to handle any int number
        ChecklExpandBuffer(16);

        // Handle the negative case
        if (integer_value < 0)
        {
            integer_value = -integer_value;
            m_buffer[m_bufferPos++] = '-';
        }

        // Copy the digits in reverse order
        int nbChars = 0;
        do
        {
            m_buffer[m_bufferPos++] = (char)('0' + integer_value % 10);
            integer_value /= 10;
            nbChars++;
        } while (integer_value != 0);

        // Reverse the result
        for (int i = nbChars / 2 - 1; i >= 0; i--)
        {
            char c = m_buffer[m_bufferPos - i - 1];
            m_buffer[m_bufferPos - i - 1] = m_buffer[m_bufferPos - nbChars + i];
            m_buffer[m_bufferPos - nbChars + i] = c;
        }

        m_isStringGenerated = false;

        return this;
    }

    ///<summary>
    /// Append a float without memory allocation.
    ///</summary>
    private FastString Append_Float(double float_value)
    {
        m_isStringGenerated = false;
        ChecklExpandBuffer(32); // Check we have enough buffer allocated to handle any float number

        // Handle the 0 case
        if (float_value == 0)
        {
            m_buffer[m_bufferPos++] = '0';
            return this;
        }

        // Handle the negative case
        if (float_value < 0)
        {
            float_value = -float_value;
            m_buffer[m_bufferPos++] = '-';
        }

        // Get the 7 meaningful digits as a long
        int nbDecimals = 0;
        while (float_value < 1000000)
        {
            float_value *= 10;
            nbDecimals++;
        }

        long valueLong = (long)System.Math.Round(float_value);

        // Parse the number in reverse order
        int nbChars = 0;
        bool isLeadingZero = true;
        while (valueLong != 0 || nbDecimals >= 0)
        {
            // We stop removing leading 0 when non-0 or decimal digit
            if (valueLong % 10 != 0 || nbDecimals <= 0)
                isLeadingZero = false;

            // Write the last digit (unless a leading zero)
            if (!isLeadingZero)
                m_buffer[m_bufferPos + (nbChars++)] = (char)('0' + valueLong % 10);

            // Add the decimal point
            if (--nbDecimals == 0 && !isLeadingZero)
                m_buffer[m_bufferPos + (nbChars++)] = '.';

            valueLong /= 10;
        }

        m_bufferPos += nbChars;

        // Reverse the result
        for (int i = nbChars / 2 - 1; i >= 0; i--)
        {
            char c = m_buffer[m_bufferPos - i - 1];
            m_buffer[m_bufferPos - i - 1] = m_buffer[m_bufferPos - nbChars + i];
            m_buffer[m_bufferPos - nbChars + i] = c;
        }

        return this;
    }

    /// <summary>
    /// Replace string
    /// </summary>
    private FastString ReplaceInternal(string matchString, IEnumerable<char> replaceString, int replaceLength)
    {
        if (m_bufferPos == 0)
            return this;

        // Create the new string into m_replacement
        for (int i = 0; i < m_bufferPos; i++)
        {
            bool isToReplace = false;
            if (m_buffer[i] == matchString[0]) // If first character found, check for the rest of the string to replace
            {
                int k = 1;

                while (k < matchString.Length && m_buffer[i + k] == matchString[k]) k++;

                isToReplace = (k >= matchString.Length);
            }

            if (isToReplace) // Do the replacement
            {
                i += matchString.Length - 1;

                if (replaceString != null)
                {
                    // TODO lxh: Code like this just for avoiding using 'delegate' which can cause GC
                    if (replaceString is string)
                    {
                        m_replacement.AddRange(replaceString);
                    }
                    else if (replaceString is char[])
                    {
                        char[] replaceCharArray = replaceString as char[];

                        for (int index = 0; index < replaceLength; index++)
                        {
                            m_replacement.Add(replaceCharArray[index]);
                        }
                    }
                }
            }
            else // No replacement, copy the old character
            {
                m_replacement.Add(m_buffer[i]);
            }
        }

        // Copy back the new string into m_chars
        ChecklExpandBuffer(m_replacement.Count - m_bufferPos);
        for (int k = 0; k < m_replacement.Count; k++)
            m_buffer[k] = m_replacement[k];

        m_bufferPos = m_replacement.Count;
        m_replacement.Clear();
        m_isStringGenerated = false;

        return this;
    }

    /// <summary>
    /// Check if we should expand the char buffer
    /// </summary>
    private void ChecklExpandBuffer(int nbCharsToAdd)
    {
        if (m_bufferPos + nbCharsToAdd > m_charsCapacity)
        {
            m_charsCapacity = Math.Max(m_charsCapacity + nbCharsToAdd, m_charsCapacity * 2);
            char[] newChars = new char[m_charsCapacity];
            m_buffer.CopyTo(newChars, 0);
            m_buffer = newChars;
        }
    }

    ///<summary>
    /// Return the string itself
    ///</summary>
    private string ToNormalString()
    {
        if (!m_isStringGenerated) // Regenerate the immutable string if needed
        {
            m_generatedString = new string(m_buffer, 0, m_bufferPos);
            m_isStringGenerated = true;
        }

        return m_generatedString;
    }
    #endregion
}