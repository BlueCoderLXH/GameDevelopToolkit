using System;

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

    //public static explicit operator FastString(string str)
    //{
    //    //var fastStr = Pool.Take();
    //    //fastStr.Append(str);

    //    return new FastString(str);
    //}

    public static implicit operator FastString(string str_value) => Pool.Take().Append(str_value);

    public static implicit operator FastString(byte byte_value) => Pool.Take().Append(byte_value);
    public static implicit operator FastString(short short_value) => Pool.Take().Append(short_value);
    public static implicit operator FastString(int int_value) => Pool.Take().Append(int_value);
    public static implicit operator FastString(long long_value) => Pool.Take().Append(long_value);
    public static implicit operator FastString(float float_value) => Pool.Take().Append(float_value);
    public static implicit operator FastString(double double_value) => Pool.Take().Append(double_value);
}