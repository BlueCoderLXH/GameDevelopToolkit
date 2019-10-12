using Framework.DesignPattern;

public partial class FastString
{
    public static ObjectPool<FastString> Pool = new ObjectPool<FastString>(() => { return new FastString(); });

    public static FastString Format(FastString fs1)
    {
        return fs1;
    }

    public static FastString Format(FastString fs1, FastString fs2)
    {
        fs1.Append(fs2);
        return fs1;
    }

    public static FastString Format(FastString fs1, FastString fs2, FastString fs3)
    {
        fs1.Append(fs2).Append(fs3);
        return fs1;
    }

    public static FastString Format(FastString fs1, FastString fs2, FastString fs3, FastString fs4)
    {
        fs1.Append(fs2).Append(fs3).Append(fs4);
        return fs1;
    }

    public static FastString Format(FastString fs1, FastString fs2, FastString fs3, FastString fs4, FastString fs5)
    {
        fs1.Append(fs2).Append(fs3).Append(fs4).Append(fs5);
        return fs1;
    }

    public static FastString Format(FastString fs1, FastString fs2, FastString fs3, FastString fs4, FastString fs5, FastString fs6)
    {
        fs1.Append(fs2).Append(fs3).Append(fs4).Append(fs5).Append(fs6);
        return fs1;
    }
}