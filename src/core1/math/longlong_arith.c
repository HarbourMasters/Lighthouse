// BanjoDecomp: (unknown decomp origin)
/* MSVC provides these as compiler intrinsics, so only define them for other compilers */
#ifndef _MSC_VER

unsigned long long __ull_rshift(unsigned long long a0, unsigned long long a1)
{
    return a0 >> a1;
}

unsigned long long __ll_lshift(unsigned long long a0, unsigned long long a1)
{
    return a0 << a1;
}

long long __ll_rshift(long long a0, long long a1)
{
    return a0 >> a1;
}

#endif /* _MSC_VER */

/* These are not MSVC intrinsics, so define them for all compilers */
unsigned long long __ull_rem(unsigned long long a0, unsigned long long a1)
{
    return a0 % a1;
}

unsigned long long __ull_div(unsigned long long a0, unsigned long long a1)
{
    return a0 / a1;
}

long long __ll_rem(unsigned long long a0, long long a1)
{
    return a0 % a1;
}

long long __ll_div(long long a0, long long a1)
{
    return a0 / a1;
}

unsigned long long __ll_mul(unsigned long long a0, unsigned long long a1)
{
    return a0 * a1;
}

void __ull_divremi(unsigned long long *div, unsigned long long *rem, unsigned long long a2, unsigned short a3)
{
    *div = a2 / a3;
    *rem = a2 % a3;
}

long long __ll_mod(long long a0, long long a1)
{
    long long tmp = a0 % a1;
    if ((tmp < 0 && a1 > 0) || (tmp > 0 && a1 < 0))
    {

        tmp += a1;
    }
    return tmp;
}
