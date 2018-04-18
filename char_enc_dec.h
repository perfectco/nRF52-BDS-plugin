/*
Copyright (c) 2017, Nordic Semiconductor ASA

All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form, except as embedded into a Nordic
   Semiconductor ASA integrated circuit in a product or a software update for
   such product, must reproduce the above copyright notice, this list of
   conditions and the following disclaimer in the documentation and/or other
   materials provided with the distribution.

3. Neither the name of Nordic Semiconductor ASA nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

4. This software, with or without modification, must only be used with a
   Nordic Semiconductor ASA integrated circuit.

5. Any software provided in binary form under this license must not be reverse
   engineered, decompiled, modified and/or disassembled.

THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef CHAR_ENC_DEC_H__
#define CHAR_ENC_DEC_H__
---

#include <stdint.h>
#include <stdbool.h>

---

/** @brief Holds information about destination buffer used during data encoding.
 */
typedef struct
{
    uint8_t * current; /**< @brief Pointer to data buffer. */
    uint8_t * end;     /**< @brief Pointer to the end fo the buffer. */
    <% if (options.bitOperations) { %>
        uint8_t freeBits;  /**< @brief Number of unused bits in the previous byte. */
    <% } %>
} bds_char_enc_buffer_t;

---

/** @brief Holds information about source buffer used during data decoding.
 */
typedef struct
{
    const uint8_t * current; /**< @brief Pointer to data buffer. */
    const uint8_t * end;     /**< @brief Pointer to the end fo the buffer. */
    <% if (options.bitOperations) { %>
        uint8_t oldBits;         /**< @brief Number of unread bits in the previous byte. */
    <% } %>
} bds_char_dec_buffer_t;

---

/** @brief Represents SFLOAT-Type from IEEE 11073.
 * ---
 * Special values:
 * --- - NaN: exponent 0, mantissa +(2^11 -1) = 0x07FF
 * --- - NRes: exponent 0, mantissa -(2^11) = 0x0800
 * --- - +INFINITY: exponent 0, mantissa +(2^11 -2) = 0x07FE
 * --- - -INFINITY: exponent 0, mantissa -(2^11 -2) = 0x0802
 */
typedef struct
{
    int16_t mantissa; /**< @brief 12-bit signed mantissa [-2048 .. 2047]. */
    int8_t exponent;  /**< @brief 4-bit signed exponent [-8 .. 7] base 10. */
} bds_ieee11073sfloat_t;

---

/** @brief Represents FLOAT-Type from IEEE 11073.
 * ---
 * Special values:
 * --- - NaN: exponent 0, mantissa +(2^23 -1) = 0x007FFFFF
 * --- - NRes: exponent 0, mantissa -(2^23) = 0x00800000
 * --- - +INFINITY: exponent 0, mantissa +(2^23 -2) = 0x007FFFFE
 * --- - -INFINITY: exponent 0, mantissa -(2^23 -2) = 0x00800002
 */
typedef struct
{
    int32_t mantissa; /**< @brief 24-bit signed mantissa [-8388608 .. 8388607]. */
    int8_t exponent;  /**< @brief 8-bit signed exponent [-128 .. 127] base 10. */
} bds_ieee11073float_t;

---

/** @brief Represents DUINT16 from IEEE 20601.
 */
typedef struct
{
    int16_t value;    /**< @brief DUINT16 value. */
} bds_ieee20601duint16_t;

---
/** @brief Initialize encoding buffer.
 * @param enc_buffer  Pointer to the encoding buffer to initialize.
 * @param buffer      Memory for storing the encoded bytes.
 * @param buffer_size Size of buffer parameter.
 */
void bds_char_enc_buffer_init(bds_char_enc_buffer_t * enc_buffer, uint8_t * buffer, uint16_t buffer_size);

---
/** @brief Finalize encoding.
 * @param enc_buffer  Pointer to the encoding buffer to finalize.
 * @param buffer      Memory for storing the encoded bytes (must be exactly the same as provided to bds_char_enc_buffer_init() function).
 * @returns           Number of bytes encoded into the buffer or zero if the buffer was to small.
 */
uint16_t bds_char_enc_buffer_done(bds_char_enc_buffer_t * enc_buffer, uint8_t * buffer);

---
/* Functions for encoding various data types */
void bds_8bit_encode_to_buffer(bds_char_enc_buffer_t * buffer, uint8_t data);
void bds_16bit_encode_to_buffer(bds_char_enc_buffer_t * buffer, uint16_t data);
void bds_24bit_encode_to_buffer(bds_char_enc_buffer_t * buffer, uint32_t data);
void bds_s24bit_encode_to_buffer(bds_char_enc_buffer_t * buffer, int32_t data);
void bds_32bit_encode_to_buffer(bds_char_enc_buffer_t * buffer, uint32_t data);
void bds_40bit_encode_to_buffer(bds_char_enc_buffer_t * buffer, uint64_t data);
void bds_48bit_encode_to_buffer(bds_char_enc_buffer_t * buffer, uint64_t data);
void bds_s48bit_encode_to_buffer(bds_char_enc_buffer_t * buffer, int64_t data);
void bds_64bit_encode_to_buffer(bds_char_enc_buffer_t * buffer, uint64_t data);
void bds_double_encode_to_buffer(bds_char_enc_buffer_t * buffer, double data);
void bds_float_encode_to_buffer(bds_char_enc_buffer_t * buffer, float data);
void bds_utf8_encode_to_buffer(bds_char_enc_buffer_t * buffer, const char * data, uint16_t max_count);
void bds_utf16_encode_to_buffer(bds_char_enc_buffer_t * buffer, const uint16_t * data, uint16_t max_count);
void bds_8bit_array_encode_to_buffer(bds_char_enc_buffer_t * buffer, const uint8_t * data, uint16_t count);
void bds_ieee11073float_encode_to_buffer(bds_char_enc_buffer_t * buffer, const bds_ieee11073float_t * data);
void bds_ieee11073sfloat_encode_to_buffer(bds_char_enc_buffer_t * buffer, const bds_ieee11073sfloat_t * data);
void bds_ieee20601duint16_encode_to_buffer(bds_char_enc_buffer_t * buffer, const bds_ieee20601duint16_t * data);

<% if (options.bitOperations) { %>
    void bds_bool_encode_to_buffer(bds_char_enc_buffer_t * buffer, bool data);
    void bds_2bit_encode_to_buffer(bds_char_enc_buffer_t * buffer, uint8_t data);
    void bds_4bit_encode_to_buffer(bds_char_enc_buffer_t * buffer, uint8_t data);
    void bds_12bit_encode_to_buffer(bds_char_enc_buffer_t * buffer, uint16_t data);
    void bds_s12bit_encode_to_buffer(bds_char_enc_buffer_t * buffer, int16_t data);
<% } %>

---
/** @brief Initialize decoding buffer.
 * @param dec_buffer  Pointer to the decoding buffer to initialize.
 * @param buffer      Memory with bytes to decode.
 * @param buffer_size Size of buffer parameter.
 */
void bds_char_dec_buffer_init(bds_char_dec_buffer_t * dec_buffer, const uint8_t * buffer, uint16_t buffer_size);

---
/** @brief Finalize decoding.
 * @param dec_buffer  Pointer to the decoding buffer to finalize.
 * @param buffer      Memory with bytes to decode (must be exactly the same as provided to bds_char_dec_buffer_init() function).
 * @returns           Number of bytes consumed from buffer or zero if buffer contains not enough data.
 */
uint16_t bds_char_dec_buffer_done(bds_char_dec_buffer_t * dec_buffer, const uint8_t * buffer);

---
/* Functions for decoding various data types */
uint8_t bds_8bit_decode_from_buffer(bds_char_dec_buffer_t * buffer);
uint16_t bds_16bit_decode_from_buffer(bds_char_dec_buffer_t * buffer);
uint32_t bds_24bit_decode_from_buffer(bds_char_dec_buffer_t * buffer);
int32_t bds_s24bit_decode_from_buffer(bds_char_dec_buffer_t * buffer);
uint32_t bds_32bit_decode_from_buffer(bds_char_dec_buffer_t * buffer);
uint64_t bds_40bit_decode_from_buffer(bds_char_dec_buffer_t * buffer);
uint64_t bds_48bit_decode_from_buffer(bds_char_dec_buffer_t * buffer);
int64_t bds_s48bit_decode_from_buffer(bds_char_dec_buffer_t * buffer);
uint64_t bds_64bit_decode_from_buffer(bds_char_dec_buffer_t * buffer);
double bds_double_decode_from_buffer(bds_char_dec_buffer_t * buffer);
float bds_float_decode_from_buffer(bds_char_dec_buffer_t * buffer);
void bds_utf8_decode_from_buffer(bds_char_dec_buffer_t * buffer, char * data, uint16_t max_count);
void bds_utf16_decode_from_buffer(bds_char_dec_buffer_t * buffer, uint16_t * data, uint16_t max_count);
void bds_8bit_array_decode_from_buffer(bds_char_dec_buffer_t * buffer, uint8_t * data, uint16_t count);
void bds_ieee11073float_decode_from_buffer(bds_char_dec_buffer_t * buffer, bds_ieee11073float_t * data);
void bds_ieee11073sfloat_decode_from_buffer(bds_char_dec_buffer_t * buffer, bds_ieee11073sfloat_t * data);
void bds_ieee20601duint16_decode_from_buffer(bds_char_dec_buffer_t * buffer, bds_ieee20601duint16_t * data);

<% if (options.bitOperations) { %>
    bool bds_bool_decode_from_buffer(bds_char_dec_buffer_t * buffer);
    uint8_t bds_2bit_decode_from_buffer(bds_char_dec_buffer_t * buffer);
    uint8_t bds_4bit_decode_from_buffer(bds_char_dec_buffer_t * buffer);
    uint16_t bds_12bit_decode_from_buffer(bds_char_dec_buffer_t * buffer);
    int16_t bds_s12bit_decode_from_buffer(bds_char_dec_buffer_t * buffer);
<% } %>

---

#endif
