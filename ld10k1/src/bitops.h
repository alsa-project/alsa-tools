#ifndef _PZ_GENERIC_BITOPS_H_
#define _PZ_GENERIC_BITOPS_H_
/* this is from linux kernel header */
/*
 * For the benefit of those who are trying to port Linux to another
 * architecture, here are some C-language equivalents.  You should
 * recode these in the native assembly language, if at all possible.
 * To guarantee atomicity, these routines call cli() and sti() to
 * disable interrupts while they operate.  (You have to provide inline
 * routines to cli() and sti().)
 *
 * Also note, these routines assume that you have 32 bit longs.
 * You will have to change this if you are trying to port Linux to the
 * Alpha architecture or to a Cray.  :-)
 * 
 * C language equivalents written by Theodore Ts'o, 9/26/92
 */
/*
 * Converted to be independent of the size of longs.
 * Zephaniah E. Hull 2005-08-15.
 */

__inline__ int set_bit(int nr, unsigned long * addr)
{
	int	mask, retval;

	addr += nr >> (sizeof(long) + 1);
	mask = 1 << (nr & (sizeof(long) * 8 - 1));
	retval = (mask & *addr) != 0;
	*addr |= mask;
	return retval;
}

__inline__ int clear_bit(int nr, unsigned long * addr)
{
	int	mask, retval;

	addr += nr >> (sizeof(long) + 1);
	mask = 1 << (nr & (sizeof(long) * 8 - 1));
	retval = (mask & *addr) != 0;
	*addr &= ~mask;
	return retval;
}

__inline__ int test_bit(int nr, unsigned long * addr)
{
	int	mask;

	addr += nr >> (sizeof(long) + 1);
	mask = 1 << (nr & (sizeof(long) * 8 - 1));
	return ((mask & *addr) != 0);
}

#endif /* _PZ_GENERIC_BITOPS_H */
