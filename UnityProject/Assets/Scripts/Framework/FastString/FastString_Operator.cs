using System;

/// <summary>
/// Override some operator, for some convenient case
/// </summary>
public partial class FastString
{
    public static FastString operator +(FastString left, string right)
    {
        left.Append(right);
        return left;
    }

    public static FastString operator +(FastString left, bool right)
    {
        left.Append(right);
        return left;
    }

    public static FastString operator +(FastString left, byte right)
    {
        left.Append(right);
        return left;
    }
    public static FastString operator +(FastString left, short right)
    {
        left.Append(right);
        return left;
    }
    public static FastString operator +(FastString left, int right)
    {
        left.Append(right);
        return left;
    }
    public static FastString operator +(FastString left, long right)
    {
        left.Append(right);
        return left;
    }

    public static FastString operator +(FastString left, float right)
    {
        left.Append(right);
        return left;
    }
    public static FastString operator +(FastString left, double right)
    {
        left.Append(right);
        return left;
    }

    public static implicit operator bool(FastString fastString) => fastString != null;

    public static implicit operator FastString(string str_value) => Aquire().Append(str_value);

    public static implicit operator FastString(bool bool_value) => Aquire().Append(bool_value);

    public static implicit operator FastString(byte byte_value) => Aquire().Append(byte_value);
    public static implicit operator FastString(short short_value) => Aquire().Append(short_value);
    public static implicit operator FastString(int int_value) => Aquire().Append(int_value);
    public static implicit operator FastString(long long_value) => Aquire().Append(long_value);
    public static implicit operator FastString(float float_value) => Aquire().Append(float_value);
    public static implicit operator FastString(double double_value) => Aquire().Append(double_value);
}