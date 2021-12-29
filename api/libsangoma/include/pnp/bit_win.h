//
//Bit manipulation macros
//

#ifndef _BIT_WIN_H_
#define _BIT_WIN_H_


/* internal macros */
				/* byte of the bitstring bit is in */
#define	_bit_byte(bit) \
	((bit) >> 3)

				/* mask for the bit within its byte */
#define	_bit_mask(bit) \
	(1 << ((bit)&0x7))

/* external macros */
				/* bytes in a bitstring of nbits bits */
#define	bitstr_size(nbits) \
	((((nbits) - 1) >> 3) + 1)

				/* is bit N of bitstring name set? */
#define	bit_test(name, bit) \
	((name)[_bit_byte(bit)] & _bit_mask(bit))

				/* set bit N of bitstring name */
#define	bit_set(name, bit) \
	(name)[_bit_byte(bit)] |= _bit_mask(bit)

				/* clear bit N of bitstring name */
#define	bit_clear(name, bit) \
	(name)[_bit_byte(bit)] &= ~_bit_mask(bit)


#define	test_bit(bit, name)		bit_test((unsigned char*)name, bit)
//#define	set_bit(bit, name)		bit_set((unsigned char*)name, bit)
//#define clear_bit(bit, name)	bit_clear((unsigned char*)name, bit)

static __inline void set_bit(int bit, void* name)
{
	bit_set((unsigned char*)name, bit);
}

static __inline int test_and_set_bit(int bit, void* name)
{
	if(test_bit(bit, name)){
		//bit already set
		return 1;
	}else{
		//not set. set it.
		bit_set((unsigned char*)name, bit);
		return 0;
	}
}

static __inline void clear_bit(int bit, void* name)
{
	bit_clear((unsigned char*)name, bit);
}

#endif //_BIT_WIN_H_