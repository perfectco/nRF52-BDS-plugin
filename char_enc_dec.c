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
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "char_enc_dec.h"

---
/** @brief Reserve bytes in encoding buffer.
 *  @param enc_buffer  Buffer where bytes will be reserved. Buffer pointer will be moved to byte just after reserved area.
 *  @param num         Number of bytes to reserve.
 *  @returns           Pointer to the beginning of the reserved bytes or NULL if not enough space in the buffer.
 */
static uint8_t* reserve_buffer(bds_char_enc_buffer_t * enc_buffer, uint16_t num)
{
    uint8_t* result = enc_buffer->current;
    uint8_t* next = enc_buffer->current + num;
    <% if (options.bitOperations) { %>
        enc_buffer->freeBits = 0;
    <% } %>
    if (next <= enc_buffer->end)
    {
        enc_buffer->current = next;
    }
    else
    {
        enc_buffer->current = enc_buffer->end + 1;
        result = NULL;
    }
    return result;
}

---
/** @brief Read bytes from decoding buffer.
 *  @param enc_buffer  Buffer to read from. Buffer pointer will be moved to byte just after read area.
 *  @param num         Number of bytes to read.
 *  @returns           Pointer to the beginning of the read bytes or NULL if not enough space in the buffer.
 */
static const uint8_t* read_buffer(bds_char_dec_buffer_t * dec_buffer, uint16_t num)
{
    const uint8_t* result = dec_buffer->current;
    const uint8_t* next = dec_buffer->current + num;
    <% if (options.bitOperations) { %>
        dec_buffer->oldBits = 0;
    <% } %>
    if (next <= dec_buffer->end)
    {
        dec_buffer->current = next;
    }
    else
    {
        dec_buffer->current = dec_buffer->end + 1;
        result = NULL;
    }
    return result;
}

---

void bds_char_enc_buffer_init(bds_char_enc_buffer_t * enc_buffer, uint8_t * buffer, uint16_t buffer_size)
{
    enc_buffer->current = buffer;
    enc_buffer->end = buffer + buffer_size;
    <% if (options.bitOperations) { %>
        enc_buffer->freeBits = 0;
    <% } %>
}

---

uint16_t bds_char_enc_buffer_done(bds_char_enc_buffer_t * enc_buffer, uint8_t * buffer)
{
    return (enc_buffer->current > enc_buffer->end) ? 0 : (uint16_t)(enc_buffer->current - buffer);
}

---

void bds_8bit_encode_to_buffer(bds_char_enc_buffer_t * buffer, uint8_t data)
{
    uint8_t* ptr = reserve_buffer(buffer, 1);
    if (ptr)
    {
        ptr[0] = data;
    }
}

---

void bds_16bit_encode_to_buffer(bds_char_enc_buffer_t * buffer, uint16_t data)
{
    uint8_t* ptr = reserve_buffer(buffer, 2);
    if (ptr)
    {
        ptr[0] = data & 0xFF;
        ptr[1] = data >> 8;
    }
}

---

void bds_24bit_encode_to_buffer(bds_char_enc_buffer_t * buffer, uint32_t data)
{
    uint8_t* ptr = reserve_buffer(buffer, 3);
    if (ptr)
    {
        ptr[0] = data & 0xFF;
        ptr[1] = (data >> 8) & 0xFF;
        ptr[2] = (data >> 16) & 0xFF;
    }
}

---

void bds_s24bit_encode_to_buffer(bds_char_enc_buffer_t * buffer, int32_t data)
{
    bds_24bit_encode_to_buffer(buffer, data);
}

---

void bds_32bit_encode_to_buffer(bds_char_enc_buffer_t * buffer, uint32_t data)
{
    uint8_t* ptr = reserve_buffer(buffer, 4);
    if (ptr)
    {
        ptr[0] = data & 0xFF;
        ptr[1] = (data >> 8) & 0xFF;
        ptr[2] = (data >> 16) & 0xFF;
        ptr[3] = data >> 24;
    }
}

---

void bds_40bit_encode_to_buffer(bds_char_enc_buffer_t * buffer, uint64_t data)
{
    bds_32bit_encode_to_buffer(buffer, data);
    bds_8bit_encode_to_buffer(buffer, data >> 32);
}

---

void bds_48bit_encode_to_buffer(bds_char_enc_buffer_t * buffer, uint64_t data)
{
    bds_32bit_encode_to_buffer(buffer, data);
    bds_16bit_encode_to_buffer(buffer, data >> 32);
}

---

void bds_s40bit_encode_to_buffer(bds_char_enc_buffer_t * buffer, int64_t data)
{
    bds_48bit_encode_to_buffer(buffer, data);
}

---

void bds_64bit_encode_to_buffer(bds_char_enc_buffer_t * buffer, uint64_t data)
{
    bds_32bit_encode_to_buffer(buffer, data);
    bds_32bit_encode_to_buffer(buffer, data >> 32);
}

---

void bds_double_encode_to_buffer(bds_char_enc_buffer_t * buffer, double data)
{
    union
    {
        double value;
        uint64_t bytes;
    } converter;
    converter.value = data;
    bds_64bit_encode_to_buffer(buffer, converter.bytes);
}

---

void bds_float_encode_to_buffer(bds_char_enc_buffer_t * buffer, float data)
{
    union
    {
        float value;
        uint32_t bytes;
    } converter;
    converter.value = data;
    bds_32bit_encode_to_buffer(buffer, converter.bytes);
}

---

void bds_utf8_encode_to_buffer(bds_char_enc_buffer_t * buffer, const char * data, uint16_t max_count)
{
    uint8_t* ptr;
    uint16_t count = 0;
    const char * str = data;
    while (*str && count + 1 < max_count)
    {
        str++;
        count++;
    }
    ptr = reserve_buffer(buffer, count);
    if (ptr)
    {
        memcpy(ptr, data, count);
    }
}

---

void bds_utf16_encode_to_buffer(bds_char_enc_buffer_t * buffer, const uint16_t * data, uint16_t max_count)
{
    uint8_t * ptr;
    uint16_t count = 1;
    const uint16_t * str = data;
    while (*str && count < max_count)
    {
        str++;
        count++;
    }
    ptr = reserve_buffer(buffer, 2 * count);
    if (ptr)
    {
        memcpy(ptr, data, 2 * count - 2);
        ptr[2 * count - 2] = 0;
        ptr[2 * count - 1] = 0;
    }
}

---

void bds_8bit_array_encode_to_buffer(bds_char_enc_buffer_t * buffer, const uint8_t * data, uint16_t count)
{
    uint8_t* ptr = reserve_buffer(buffer, count);
    if (ptr)
    {
        memcpy(ptr, data, count);
    }
}

---

void bds_ieee11073float_encode_to_buffer(bds_char_enc_buffer_t * buffer, const bds_ieee11073float_t * data)
{
    bds_24bit_encode_to_buffer(buffer, data->mantissa);
    bds_8bit_encode_to_buffer(buffer, data->exponent);
}

---

void bds_ieee11073sfloat_encode_to_buffer(bds_char_enc_buffer_t * buffer, const bds_ieee11073sfloat_t * data)
{
    uint16_t sfloat = (data->mantissa & 0x0FFF) | ((uint16_t)data->exponent << 12);
    bds_16bit_encode_to_buffer(buffer, sfloat);
}

---

void bds_ieee20601duint16_encode_to_buffer(bds_char_enc_buffer_t * buffer, const bds_ieee20601duint16_t * data)
{
    bds_16bit_encode_to_buffer(buffer, data->value);
}

---

<% if (options.bitOperations) { %>

    /** @brief Encode data on bit level.
    *  @param enc_buffer  Buffer for writing.
    *  @param data        Lower bits from this parameter will be encoded.
    *  @param bits        Number of bits to encode (from 1 to 7).
    */
    static void bits_encode_to_buffer(bds_char_enc_buffer_t * buffer, uint8_t data, uint8_t bits)
    {
        data &= (1 << bits) - 1;
        if (buffer->freeBits >= bits)
        {
            buffer->current[-1] |= (data << (8 - buffer->freeBits));
            buffer->freeBits -= bits;
        }
        else
        {
            uint8_t freeBits = buffer->freeBits;
            uint8_t * ptr = reserve_buffer(buffer, 1);
            if (ptr)
            {
                if (freeBits > 0)
                {
                    ptr[-1] |= (data << (8 - freeBits));
                    data >>= freeBits;
                }
                ptr[0] = data;
                buffer->freeBits = 8 + freeBits - bits;
            }
        }
    }

    ---

    void bds_bool_encode_to_buffer(bds_char_enc_buffer_t * buffer, bool data)
    {
        bits_encode_to_buffer(buffer, (data ? 1 : 0), 1);
    }

    ---

    void bds_2bit_encode_to_buffer(bds_char_enc_buffer_t * buffer, uint8_t data)
    {
        bits_encode_to_buffer(buffer, data, 2);
    }

    ---

    void bds_4bit_encode_to_buffer(bds_char_enc_buffer_t * buffer, uint8_t data)
    {
        bits_encode_to_buffer(buffer, data, 4);
    }

    ---

    void bds_12bit_encode_to_buffer(bds_char_enc_buffer_t * buffer, uint16_t data)
    {
        bits_encode_to_buffer(buffer, data & 0x3F, 6);
        bits_encode_to_buffer(buffer, (data >> 6) & 0x3F, 6);
    }

    ---

    void bds_s12bit_encode_to_buffer(bds_char_enc_buffer_t * buffer, int16_t data)
    {
        bds_12bit_encode_to_buffer(buffer, data);
    }

    ---

<% } %>


void bds_char_dec_buffer_init(bds_char_dec_buffer_t * dec_buffer, const uint8_t * buffer, uint16_t buffer_size)
{
    dec_buffer->current = buffer;
    dec_buffer->end = buffer + buffer_size;
    <% if (options.bitOperations) { %>
        dec_buffer->oldBits = 0;
    <% } %>
}

---

uint16_t bds_char_dec_buffer_done(bds_char_dec_buffer_t * dec_buffer, const uint8_t * buffer)
{
    return (dec_buffer->current > dec_buffer->end) ? 0 : (uint16_t)(dec_buffer->current - buffer);
}

---

uint8_t bds_8bit_decode_from_buffer(bds_char_dec_buffer_t * buffer)
{
    const uint8_t* ptr = read_buffer(buffer, 1);
    if (ptr)
    {
        return ptr[0];
    }
    return 0;
}

---

uint16_t bds_16bit_decode_from_buffer(bds_char_dec_buffer_t * buffer)
{
    const uint8_t* ptr = read_buffer(buffer, 2);
    if (ptr)
    {
        return (uint16_t)ptr[0] | ((uint16_t)ptr[1] << 8);
    }
    return 0;
}

---

uint32_t bds_24bit_decode_from_buffer(bds_char_dec_buffer_t * buffer)
{
    const uint8_t* ptr = read_buffer(buffer, 3);
    if (ptr)
    {
        return (uint32_t)ptr[0] | ((uint32_t)ptr[1] << 8) | ((uint32_t)ptr[2] << 16);
    }
    return 0;
}

---

int32_t bds_s24bit_decode_from_buffer(bds_char_dec_buffer_t * buffer)
{
    int32_t result = bds_24bit_decode_from_buffer(buffer);
    if (result & 0x00800000)
    {
        result |= 0xFF000000;
    }
    return result;
}

---

uint32_t bds_32bit_decode_from_buffer(bds_char_dec_buffer_t * buffer)
{
    const uint8_t* ptr = read_buffer(buffer, 4);
    if (ptr)
    {
        return (uint32_t)ptr[0] | ((uint32_t)ptr[1] << 8) | ((uint32_t)ptr[2] << 16) | ((uint32_t)ptr[3] << 24);
    }
    return 0;
}

---

uint64_t bds_40bit_decode_from_buffer(bds_char_dec_buffer_t * buffer)
{
    uint64_t low = bds_32bit_decode_from_buffer(buffer);
    uint64_t high = bds_8bit_decode_from_buffer(buffer);
    return low | (high << 32);
}

---

uint64_t bds_48bit_decode_from_buffer(bds_char_dec_buffer_t * buffer)
{
    uint64_t low = bds_32bit_decode_from_buffer(buffer);
    uint64_t high = bds_16bit_decode_from_buffer(buffer);
    return low | (high << 32);
}

---

int64_t bds_s48bit_decode_from_buffer(bds_char_dec_buffer_t * buffer)
{
    int64_t result = bds_48bit_decode_from_buffer(buffer);
    if (result & ((int64_t)1 << 47))
    {
        result |= ~(((int64_t)1 << 48) - 1);
    }
    return result;
}

---

uint64_t bds_64bit_decode_from_buffer(bds_char_dec_buffer_t * buffer)
{
    uint64_t low = bds_32bit_decode_from_buffer(buffer);
    uint64_t high = bds_32bit_decode_from_buffer(buffer);
    return low | (high << 32);
}

---

double bds_double_decode_from_buffer(bds_char_dec_buffer_t * buffer)
{
    union
    {
        double value;
        uint64_t bytes;
    } converter;
    converter.bytes = bds_64bit_decode_from_buffer(buffer);
    return converter.value;
}

---

float bds_float_decode_from_buffer(bds_char_dec_buffer_t * buffer)
{
    union
    {
        float value;
        uint32_t bytes;
    } converter;
    converter.bytes = bds_32bit_decode_from_buffer(buffer);
    return converter.value;
}

---

void bds_utf8_decode_from_buffer(bds_char_dec_buffer_t * buffer, char * data, uint16_t max_count)
{
    while (buffer->current < buffer->end && *buffer->current)
    {
        if (max_count <= 1)
        {
            // string too long - interrupt parsing and report error at the end
            buffer->current = buffer->end + 1;
            return;
        }
        *data = *buffer->current;
        data++;
        buffer->current++;
        max_count--;
    }
    *data = 0;
}

---

void bds_utf16_decode_from_buffer(bds_char_dec_buffer_t * buffer, uint16_t * data, uint16_t max_count)
{
    while (buffer->current + 1 < buffer->end && (buffer->current[0] || buffer->current[1]))
    {
        if (max_count <= 1)
        {
            // string too long - interrupt parsing and report error at the end
            buffer->current = buffer->end + 1;
            return;
        }
        *data = (uint16_t)buffer->current[0] | ((uint16_t)buffer->current[1] << 8);
        data++;
        buffer->current += 2;
        max_count--;
    }
    *data = 0;
}

---

void bds_8bit_array_decode_from_buffer(bds_char_dec_buffer_t * buffer, uint8_t * data, uint16_t count)
{
    const uint8_t* ptr = read_buffer(buffer, count);
    if (ptr)
    {
        memcpy(data, ptr, count);
    }
}

---

void bds_ieee11073float_decode_from_buffer(bds_char_dec_buffer_t * buffer, bds_ieee11073float_t * data)
{
    data->mantissa = bds_s24bit_decode_from_buffer(buffer);
    data->exponent = bds_8bit_decode_from_buffer(buffer);
}

---

void bds_ieee11073sfloat_decode_from_buffer(bds_char_dec_buffer_t * buffer, bds_ieee11073sfloat_t * data)
{
    uint16_t value = bds_16bit_decode_from_buffer(buffer);
    data->mantissa = value & 0x0FFF;
    if (data->mantissa & 0x0800)
    {
        data->mantissa |= 0xF000;
    }
    data->exponent = value >> 12;
    if (data->exponent & 0x8)
    {
        data->exponent |= 0xF0;
    }
}

---

void bds_ieee20601duint16_decode_from_buffer(bds_char_dec_buffer_t * buffer, bds_ieee20601duint16_t * data)
{
    data->value = bds_16bit_decode_from_buffer(buffer);
}

---

<% if (options.bitOperations) { %>

    /** @brief Decode data on bit level.
    *  @param enc_buffer  Buffer for reading.
    *  @param bits        Number of bits to decode (from 1 to 7).
    *  @returns           Decoded bits in lower bits, unused upper bits are zero.
    */
    static uint8_t bits_decode_from_buffer(bds_char_dec_buffer_t * buffer, uint8_t bits)
    {
        uint8_t result = 0;
        if (buffer->oldBits >= bits)
        {
            result = (buffer->current[-1] >> (8 - buffer->oldBits));
            buffer->oldBits -= bits;
        }
        else
        {
            uint8_t oldBits = buffer->oldBits;
            const uint8_t* ptr = read_buffer(buffer, 1);
            if (ptr)
            {
                result = ptr[0];
                if (oldBits > 0)
                {
                    result <<= oldBits;
                    result |= (ptr[-1] >> (8 - oldBits));
                }
                buffer->oldBits = 8 + oldBits - bits;
            }
        }
        result &= (1 << bits) - 1;
        return result;
    }

    ---

    bool bds_bool_decode_from_buffer(bds_char_dec_buffer_t * buffer)
    {
        return bits_decode_from_buffer(buffer, 1) ? true : false;
    }

    ---

    uint8_t bds_2bit_decode_from_buffer(bds_char_dec_buffer_t * buffer)
    {
        return bits_decode_from_buffer(buffer, 2);
    }

    ---

    uint8_t bds_4bit_decode_from_buffer(bds_char_dec_buffer_t * buffer)
    {
        return bits_decode_from_buffer(buffer, 4);
    }

    ---

    uint16_t bds_12bit_decode_from_buffer(bds_char_dec_buffer_t * buffer)
    {
        uint16_t low = bits_decode_from_buffer(buffer, 6);
        uint16_t high = bits_decode_from_buffer(buffer, 6);
        return low | (high << 6);
    }

    ---

    int16_t bds_s12bit_decode_from_buffer(bds_char_dec_buffer_t * buffer)
    {
        int16_t result = bds_12bit_decode_from_buffer(buffer);
        if (result & 0x0800)
        {
            result |= 0xF000;
        }
        return result;
    }

    ---

<% } %>




