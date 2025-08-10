/***************************************************************************
 *   Copyright (C) 2013-2025 by Guillermo Ballester Valor                  *
 *   gbv@ogimet.com                                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
/*!
 \file bufrdeco_utils.c
 \brief This file has the code of useful routines for library bufrdeco
*/
#include "bufrdeco.h"

uint8_t bitf[8] = {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01}; /*!< Mask a single bit of a byte */
uint8_t biti[8] = {0xFF,0x7f,0x3f,0x1F,0x0F,0x07,0x03,0x01}; /*!< Mask remaining bits in a byte (less significant) */
uint8_t bitk[8] = {0x80,0xc0,0xe0,0xf0,0xf8,0xfc,0xfe,0xff}; /*!< Mask first bits in a byte (most significant) */

/*!
  \fn size_t get_bits_as_char_array ( char *target, uint8_t *has_data, uint8_t *source, size_t *bit0_offset, size_t bit_length )
  \brief Get a string from an array of bits
  \param target string as result
  \param has_data Output flags to check whether is missing data. If 0 then data is missing, othewise has data
  \param source array of uint8_t elements. Most significant bit of first element is the bit offest reference
  \param bit0_offset Bit offset
  \param bit_length Lenght (in bits) for the chunck to extract
  \return If returns the amount of bits readed. 0 if problems. It also update bits_offset with the new bits.
*/
buf_t get_bits_as_char_array2 ( char *target, uint8_t *has_data, uint8_t *source, buf_t *bit0_offset, buf_t bit_length )
{
  buf_t i, j;
  buf_t nc;

  bufrdeco_assert (has_data != NULL && source != NULL && target != NULL && bit0_offset != NULL);

  if ( bit_length % 8 )
    return 0; // bit_length needs to be divisible by 8

  nc = bit_length / 8;
  *has_data = 0; // marc if no missing data is present
  for ( j = 0; j < nc ; j++ )
    {
      buf_t r = 8;
      buf_t d = 0;
      * ( target + j ) = 0;
      do
        {
          const uint8_t *c = source + ( *bit0_offset + d ) / 8;
          i = ( *bit0_offset + d ) % 8;
          if ( *c & bitf[i] )
            * ( target + j ) += ( 1U << ( r - 1 ) );
          else
            *has_data = 1;
          d += 1;
          r -= 1;
        }
      while ( r > 0 );
      *bit0_offset += 8; // update bit0_offset
    }
  * ( target + nc ) = '\0';
  return bit_length;
}

/*!
  \fn size_t get_bits_as_char_array ( char *target, uint8_t *has_data, uint8_t *source, size_t *bit0_offset, size_t bit_length )
  \brief get a sequence of bits in data section 4 from a BUFR reports to get an array of chars
  \param target string with the resulting array
  \param has_data if 1 then has data, if 0 what we got is missing data
  \param source buffer of uint8_t which is actually the data section 4.
  \param bit0_offset bit offset of first bit of first char
  \param bit_length number of bits to extract. Obviously this will be divisible by 8
  \return If returns the amount of bits readed. 0 if problems. It also update bit0_offset with the new bits.
*/
buf_t get_bits_as_char_array ( char *target, uint8_t *has_data, uint8_t *source, buf_t *bit0_offset, buf_t bit_length )
{
  buf_t i, j, k;
  buf_t nc;
  

  bufrdeco_assert (has_data != NULL && source != NULL && target != NULL && bit0_offset != NULL);

  if ( bit_length % 8 )
    return 0; // bit_length needs to be divisible by 8

  //printf("bit_length=%lu\n", bit_length);

  nc = bit_length / 8;
  i = ( *bit0_offset ) % 8;
  k = 8 - i;
  *has_data = 0; // marc if no missing data is present
  for ( j = 0; j < nc ; j++ )
    {
      const uint8_t *c = source + ( *bit0_offset )  / 8;
      * ( target + j ) = ( *c & biti[i] );
      if ( i )
        {
          * ( target + j ) <<= i;
          * ( target + j ) |= ( ( * ( c + 1 ) & bitk[i - 1] ) >> k );
        }
      if ( * ( target + j ) != (char)(-1) )
        *has_data = 1;
      //printf("%c", * ( target + j ) );
      *bit0_offset += 8; // update bit0_offset
    }
  * ( target + nc ) = '\0';
  return bit_length;
}

/*!
  \fn size_t get_bits_as_uint32_t2 ( uint32_t *target, uint8_t *has_data, uint8_t *source, buf_t *bit0_offset, buf_t bit_length )
  \brief Read bits from an array of uint8_t and set them as an uint32_t
  \param target uint32_t pointer where to set the result
  \param has_data Output flags to check whether is missing data. If 0 then data is missing, othewise has data
  \param source array of uint8_t elements. Most significant bit of first element is the bit offset reference
  \param bit0_offset Bit offset
  \param bit_length Lenght (in bits) for the chunck to extract
  \return If returns the amount of bits readed. 0 if problems. It also update bits_offset with the new bits.

  This is a version which extract bit a bit. For more than about 8 bits should be used the algorithm in
  \ref get_bits_as_uint32_t

*/
uint32_t get_bits_as_uint32_t2 ( uint32_t *target, uint8_t *has_data, uint8_t *source, buf_t *bit0_offset, buf_t bit_length )
{
  buf_t r, d;

  bufrdeco_assert (has_data != NULL && source != NULL && target != NULL && bit0_offset != NULL);

  if ( bit_length > 32 || bit_length == 0 )
    return 0;

  r = bit_length;
  d = 0;
  *target = 0;
  *has_data = 0;

  do
    {
      const uint8_t *c = source + ( *bit0_offset + d ) / 8;
      buf_t i = ( *bit0_offset + d ) % 8;
      if ( *c & bitf[i] )
        *target += ( 1U << ( r - 1 ) );
      else
        *has_data = 1;
      d += 1;
      r -= 1;
    }
  while ( r > 0 );
  *bit0_offset += bit_length; // update bit0_offset

  return bit_length;
}


/*!
  \fn size_t get_bits_as_uint32_t ( uint32_t *target, uint8_t *has_data, uint8_t *source, size_t *bit0_offset, size_t bit_length )
  \brief Read bits from an array of uint8_t and set them as an uint32_t
  \param target uint32_t pointer where to set the result
  \param has_data Output flags to check whether is missing data. If 0 then data is missing, othewise has data
  \param source array of uint8_t elements. Most significant bit of first element is the bit offset reference
  \param bit0_offset Bit offset
  \param bit_length Lenght (in bits) for the chunck to extract
  \return If returns the amount of bits readed. 0 if problems. It also update bits_offset with the new bits.

  If bil_length is less than 8 then it uses version 2 \ref get_bits_as_uint32_t2

*/
uint32_t get_bits_as_uint32_t ( uint32_t *target, uint8_t *has_data, uint8_t *source, buf_t *bit0_offset, buf_t bit_length )
{
  int i;
  uint8_t *c;
  uint64_t x;

  bufrdeco_assert (has_data != NULL && source != NULL && target != NULL && bit0_offset != NULL);

  if ( bit_length > 32 || bit_length == 0 )
    return 0;

  if ( bit_length < 8 )
    return get_bits_as_uint32_t2 ( target, has_data, source, bit0_offset, bit_length );

  *target = 0;
  *has_data = 0; // marc if no missing data is present
  c = source + ( *bit0_offset ) / 8;
  i = ( *bit0_offset ) % 8;
  x = ( ( uint64_t ) ( *c & biti[i] ) << 32 ) + ( ( uint64_t ) ( * ( c + 1 ) ) << 24 ) + ( ( uint64_t ) ( * ( c + 2 ) ) << 16 ) +
      ( ( uint64_t ) ( * ( c + 3 ) ) << 8 ) + ( uint64_t ) ( * ( c + 4 ) ); // 40 - i bits
  x >>= ( 40 - i - bit_length );
  *target = ( uint32_t ) x;
  if ( ( 1UL << bit_length ) != ( x + 1UL ) )
    *has_data = 1;

  *bit0_offset += bit_length; // update bit0_offset
  return bit_length;
}


/*!
 \fn int get_table_b_reference_from_uint32_t ( int32_t *target, uint8_t bits, uint32_t source )
 \brief Get an int32_t from bits according with bufr criteria to change the reference of a descritor. Most significant bit in source is sign
 \param target int32_t as result
 \param bits number of bits to consider
 \param source uint32_T with the data to transform
 \return If success return 0, 1 otherwise
*/
// most significant of bits is the sign
int get_table_b_reference_from_uint32_t ( int32_t *target, uint8_t bits, uint32_t source )
{
  uint32_t mask = 1;

  bufrdeco_assert (target != NULL);


  if ( bits > 32 || bits == 0 )
    return 1;

  if ( bits > 1 )
    mask = ( ( uint32_t ) 1 << ( bits - 1 ) );

  if ( mask & source )
    {
      // case of negative number
      *target = - ( int32_t ) ( source - mask );
    }
  else
    *target = ( int32_t ) ( source );
  return 0;
}

/*!
  \fn uint32_t two_bytes_to_uint32(const uint8_t *source)
  \brief returns the uint32_t value from an array of two bytes, most significant first
  \param source pointer to source uint8_t
  \return the uint32_t resulting
*/
uint32_t two_bytes_to_uint32 ( const uint8_t *source )
{

  bufrdeco_assert (source != NULL);

  return ( ( uint32_t ) source[1] + ( uint32_t ) source[0] * 256 );
}


/*!
  \fn uint32_t three_bytes_to_uint32(const uint8_t *source)
  \brief returns the uint32_t value from an array of three bytes, most significant first
  \param source pointer to source uint8_t
  \return the uint32_t resulting
*/
uint32_t three_bytes_to_uint32 ( const uint8_t *source )
{
  bufrdeco_assert (source != NULL);
  return ( ( uint32_t ) source[2] + ( uint32_t ) source[1] * 256 + ( uint32_t ) source[0] * 65536 );
}

/*!
  \fn int uint32_t_to_descriptor(struct bufr_descriptor *d, uint32_t id)
  \brief parse an integer with a descriptor fom bufr ECWMF libary
  \param d pointer to a struct \ref bufr_descriptor where to set the result on output
  \param id integer with the descriptor from ewcwf
  \return the resulting uint32_t
*/
int uint32_t_to_descriptor ( struct bufr_descriptor *d, uint32_t id )
{
  bufrdeco_assert (d != NULL);

  d->f = id / 100000;
  d->x = ( id % 100000 ) / 1000;
  d->y = id % 1000;
  sprintf ( d->c, "%06u", id );
  return 0;
}

/*!
  \fn int two_bytes_to_descriptor (struct bufr_descriptor *d, const uint8_t *source)
  \brief set a struct \ref bufr_descriptor from two consecutive bytes in bufr file
  \param source pointer to first byte (most significant)
  \param d pointer to the resulting descriptor

  \return 0 if all is OK. 1 otherwise
 */
int two_bytes_to_descriptor ( struct bufr_descriptor *d, const uint8_t *source )
{
  bufrdeco_assert (source != NULL && d != NULL);

  d->y = source[1];
  d->x = source[0] & 0x3f;
  d->f = ( source[0] >> 6 ) & 0x03;
  sprintf ( d->c, "%u%02u%03u", d->f, d->x, d->y );
  return 0;
}

/*!
  \fn char * bufr_charray_to_string(char *s, char *buf, size_t size)
  \brief get a null termitated c-string from an array of unsigned chars
  \param s resulting string
  \param buf pointer to first element in array
  \param size number of chars in array
  \result the resulting s string
*/
char * bufr_charray_to_string ( char *s, const char *buf, size_t size )
{
  bufrdeco_assert (s != NULL && buf != NULL);

  // copy
  memcpy ( s, buf, size );
  // add final string mark
  s[size] = '\0';
  return s;
}

/*!
  \fn char * bufr_adjust_string(char *s)
  \brief Supress trailing blanks of a string
  \param s string to process
  \result the resulting s string
*/
char * bufr_adjust_string ( char *s )
{
  size_t l;

  bufrdeco_assert (s != NULL);

  l = strlen ( s );
  while ( l && s[--l] == ' ' )
    s[l] = '\0';
  return s;
}

/*!
  \fn int is_a_delayed_descriptor ( struct bufr_descriptor *d )
  \brief check if a descriptor is a delayed descriptor
  \param d pointer to a struct \ref bufr_descriptor to check
  \return If is a delayed desccriptor return 1, 0 otherwise.
*/
int is_a_delayed_descriptor ( const struct bufr_descriptor *d )
{
  bufrdeco_assert (d != NULL);

  if ( ( d->f == 0 ) &&
       ( d->x == 31 ) &&
       ( d->y == 1 || d->y == 2 || d->y == 11 || d->y == 12 ) )
    return 1;
  else
    return 0;
}

/*!
  \fn int is_a_short_delayed_descriptor ( struct bufr_descriptor *d )
  \brief check if a descriptor is a short delayed descriptor
  \param d pointer to a struct \ref bufr_descriptor to check
  \return If is a delayed descriptor return 1, 0 otherwise.
*/
int is_a_short_delayed_descriptor ( const struct bufr_descriptor *d )
{
  bufrdeco_assert (d != NULL);

  if ( ( d->f == 0 ) &&
       ( d->x == 31 ) &&
       ( d->y == 0 ) )
    return 1;
  else
    return 0;
}


/*!
  \fn int is_a_local_descriptor ( struct bufr_descriptor *d )
  \brief check if a descriptor is a local descriptor
  \param d pointer to a struct \ref bufr_descriptor to check
  \return If is a local desccriptor return 1, 0 otherwise.
*/
int is_a_local_descriptor ( const struct bufr_descriptor *d )
{
  bufrdeco_assert (d != NULL);

  if ( ( d->f == 0 ) &&
       ( d->x >= 48 ) &&
       ( d->x <= 63 ) )
    return 1;
  else
    return 0;
}

/*!
   \fn char *get_formatted_value_from_escale ( char *fmt, size_t dim, int32_t escale, double val )
   \brief gets a string with formatted value depending of scale
   \param fmt The output target string
   \param dim Size of available space (bytes) to write the result
   \param escale value scale in descriptor
   \param val double to printf
   \return the resulting fmt string
   This version use 17 width for number plus a final space
*/
char *get_formatted_value_from_escale ( char *fmt, size_t dim, int32_t escale, double val )
{
  bufrdeco_assert (fmt != NULL);

  if ( escale >= 0 )
    {
      char aux[32];
      sprintf ( aux, "%%17.%dlf ", escale );
      snprintf ( fmt, dim, aux, val );
    }
  else
    snprintf ( fmt, dim, "%17.0lf ", val );
  return fmt;
}

/*!
   \fn char *get_formatted_value_from_escale2 ( char *fmt, size_t dim, int32_t escale, double val )
   \brief gets a string with formatted value depending of scale
   \param fmt The output target string
   \param dim Size of available space (bytes) to write the result
   \param escale value scale in descriptor
   \param val double to printf
   \return the resulting fmt string

   Differs from get_formatted_value_from_escale that no blanks are written
 */
char *get_formatted_value_from_escale2 ( char *fmt, size_t dim, int32_t escale, double val )
{
  bufrdeco_assert (fmt != NULL);

  if ( escale >= 0 )
    {
      char aux[32];
      sprintf ( aux, "%%.%dlf", escale );
      snprintf ( fmt, dim, aux, val );
    }
  else
    snprintf ( fmt, dim, "%.0lf", val );
  return fmt;
}


/*!
   \fn int bufrdeco_add_to_bitmap( struct bufrdeco_bitmap *bm, buf_t index_to, buf_t index_by )
   \brief Push a bitmap element in a \ref bufrdeco_bitmap
   \param bm target struct \ref bufrdeco_bitmap where to push
   \param index_to index of the \ref bufrdeco_bitmap which this is bitmapping to
   \param index_by index of the \ref bufrdeco_bitmap which this is bitmapped by

   \return If no space to push returns 1, otherwise 0
*/
int bufrdeco_add_to_bitmap ( struct bufrdeco_bitmap *bm, buf_t index_to, buf_t index_by )
{
  bufrdeco_assert (bm != NULL);

  if ( bm->nb < BUFR_MAX_BITMAP_PRESENT_DATA )
    {
      bm->bitmap_to[bm->nb] = index_to;
      bm->me[bm->nb] = index_by;
      ( bm->nb )++;
      return 0;
    }
  return 1;
}

/*!
 * \fn int bufrdeco_get_bitmaped_info ( struct bufrdeco_bitmap_related_vars *brv, uint32_t target, struct bufrdeco *b )
 * \brief Get bitmap info searching into bitmaps
 * \param brv pointer to struct \ref bufrdeco_bitmap_related_vars where to set the results
 * \param target The key to find in array of bitmaps. It is the index of a ref in compressed case or an atom data in other case
 * \param b pointer to the current struct \ref bufrdeco
 *
 * \return If found the target return 0, othewise return 1
 */
int bufrdeco_get_bitmaped_info ( struct bufrdeco_bitmap_related_vars *brv, uint32_t target, struct bufrdeco *b )
{
  buf_t i, j, k;
  buf_t delta;
  struct bufrdeco_bitmap *bm;

  bufrdeco_assert (b != NULL && brv != NULL);
  brv->target = target;
  memset ( brv, 0, sizeof ( struct bufrdeco_bitmap_related_vars ) );
  for ( i = 0; i < b->bitmap.nba ; i++ )
    {
      bm = b->bitmap.bmap[i];
      for ( j = 0; j < bm->nb ; j++ )
        {
          if ( bm->bitmap_to[j] == target )
            {
              brv->nba = i;
              brv->nb = j;
              brv->bitmaped_by = bm->me[j]; // is the index of bit present data in ref/data
              delta = bm->me[j] - bm->me[0]; // delta is the refence with recpect the reference of first data present (bit = 0) in bitmap
              // quality data
              if ( bm->nq )
                {
                  for ( k = 0; k < bm->nq; k++ )
                    {
                      brv->qualified_by[k] = bm->quality[k] + delta; // remeber that bm->quality[k] is refered to first data_present
                    }
                }

              // substituded
              if ( bm->subs )
                brv->substituted = bm->subs + delta; // bm->subs is for first data present

              if ( bm->retain )
                brv->retained = bm->retain + delta;

              if ( bm->ns1 )
                {
                  for ( k = 0; k < bm->ns1; k++ )
                    {
                      brv->stat1[k] = bm->stat1[k] + delta;
                      brv->stat1_desc[k] = bm->stat1_desc[k];
                    }
                }

              if ( bm->nds )
                {
                  for ( k = 0; k < bm->nds; k++ )
                    {
                      brv->dstat[k] = bm->dstat[k] + delta;
                      brv->dstat_desc[k] = bm->dstat_desc[k];
                    }
                }

              return 0;
            }
          else if ( b->bitmap.bmap[i]->bitmap_to[j] > target )
            break;
        }
    }
  return 1;
}


/*!
 * \fn int bufr_write_subset_offset_bits (FILE *f , struct bufrdeco_subset_bit_offsets *off)
 * \brief Write offset bit array for subsets in a non-compressed bufr
 * \param f file pointer opened by caller
 * \param off pointer to the struct \ref bufrdeco_subset_bit_offsets with the data to write into file
 * \return if success return 0, otherwise 1
 */
int bufr_write_subset_offset_bits ( FILE *f, const struct bufrdeco_subset_bit_offsets *off )
{
  size_t wrote;

  bufrdeco_assert (off != NULL && f != NULL );

  wrote = fwrite ( & ( off->nr ), sizeof ( buf_t ), 1, f );
  bufrdeco_assert_with_return_val ( wrote == 1, 1 );

  wrote = fwrite ( & ( off->ofs[0] ), sizeof ( buf_t ), off->nr, f );
  bufrdeco_assert_with_return_val ( wrote == off->nr, 1 );

  return 0;
}

/*!
 * \fn int bufr_read_subset_offset_bits (FILE *f , struct bufrdeco_subset_bit_offsets *off)
 * \brief Write offset bit array for subsets in a non-compressed bufr
 * \param f file pointer opened by caller
 * \param off pointer to the struct \ref bufrdeco_subset_bit_offsets with the data to write into file
 * \return if success return 0, otherwise 1
 */
int bufr_read_subset_offset_bits ( FILE *f, struct bufrdeco_subset_bit_offsets *off )
{
  size_t readed;

  bufrdeco_assert (off != NULL && f != NULL );

  readed = fread ( & ( off->nr ), sizeof ( buf_t ), 1, f );
  bufrdeco_assert_with_return_val ( readed == 1, 1 );

  readed = fread ( & ( off->ofs[0] ), sizeof ( buf_t ), off->nr, f );
  bufrdeco_assert_with_return_val ( readed == off->nr, 1 );

  return 0;
}

/*!
 * \fn char *strcat_safe ( char *dst, const char *src, size_t n )
 * \brief An secure version emulation of strcat.
 */
char *strcat_safe ( char *dst, const char *src, size_t n )
{
  char *p = dst;

  while ( n != 0 && *p != '\0' )
    {
      p++;
      n--;
    }
  if ( n != 0 )
    {
      for ( ; --n != 0; p++, src++ )
        {
          if ( ( *p = *src ) == '\0' )
            return dst;
        }
      *p = '\0';
    }
  return dst;
}
