/// <summary>
/// Format FastString object, just like the string.Format
/// </summary>
public partial class FastString
{
    public static FastString Format(
        FastString format,
        FastString args0)
    {
        format.Replace("{0}", args0);

        Release(args0);

        return format;
    }

    public static FastString Format(
        FastString format,
        FastString args0,
        FastString args1)
    {
        format.Replace("{0}", args0);
        format.Replace("{1}", args1);

        Release(args0);
        Release(args1);

        return format;
    }

    public static FastString Format(
        FastString format,
        FastString args0,
        FastString args1,
        FastString args2)
    {
        format.Replace("{0}", args0);
        format.Replace("{1}", args1);
        format.Replace("{2}", args2);

        Release(args0);
        Release(args1);
        Release(args2);

        return format;
    }

    public static FastString Format(
        FastString format,
        FastString args0,
        FastString args1,
        FastString args2,
        FastString args3)
    {
        format.Replace("{0}", args0);
        format.Replace("{1}", args1);
        format.Replace("{2}", args2);
        format.Replace("{3}", args3);

        Release(args0);
        Release(args1);
        Release(args2);
        Release(args3);

        return format;
    }

    public static FastString Format(
        FastString format,
        FastString args0,
        FastString args1,
        FastString args2,
        FastString args3,
        FastString args4)
    {
        format.Replace("{0}", args0);
        format.Replace("{1}", args1);
        format.Replace("{2}", args2);
        format.Replace("{3}", args3);
        format.Replace("{4}", args4);

        Release(args0);
        Release(args1);
        Release(args2);
        Release(args3);
        Release(args4);

        return format;
    }

    public static FastString Format(
        FastString format,
        FastString args0,
        FastString args1,
        FastString args2,
        FastString args3,
        FastString args4,
        FastString args5)
    {
        format.Replace("{0}", args0);
        format.Replace("{1}", args1);
        format.Replace("{2}", args2);
        format.Replace("{3}", args3);
        format.Replace("{4}", args4);
        format.Replace("{5}", args5);

        Release(args0);
        Release(args1);
        Release(args2);
        Release(args3);
        Release(args4);
        Release(args5);

        return format;
    }
}