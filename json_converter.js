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


converter = new (function ()
{
    var th = this;


    function isNativeList(type)
    {
        if (type.IsArray) return true;
        while (type.Name !== 'Object')
        {
            if (type.Name === 'List`1') return true;
            type = type.baseType;
        }
        return false;
    }


    function convertNativeObject(obj)
    {
        var result = {};

        // Get properties of data object and iterate through
        var props = obj.GetType().GetProperties();
        for (var i = 0; i < props.length; i++)
        {
            var prop = props[i];
            // Only simple readable properties have to be interpreted and skip DropBase.Context
            if (!_.isNull(prop.GetMethod) && prop.GetMethod.GetParameters().length == 0
                    && (prop.DeclaringType.Name != 'DropBase' || prop.Name != 'Context'))
            {
                result[prop.Name] = convertItem(prop.GetValue(obj), prop.PropertyType);
            }
        }

        return result;
    }


    function convertNativeList(list)
    {
        var result = [];
        var len = _.isUndefined(list.length) ? list.Length : list.length;

        for (var i = 0; i < len; i++)
        {
            result.push(convertItem(list[i]));
        }

        return result;
    }


    function convertArray(data)
    {
        var result = data.slice();

        for (var key in data)
        {
            result[key] = convertItem(data[key]);
        }

        return result;
    }


    function convertObject(data)
    {
        var result = {};

        for (var key in data)
        {
            result[key] = convertItem(data[key]);
        }

        return result;
    }


    function convertItem(value, type)
    {
        if (typeof(value) != 'object')
        {
            // primitive type
            return value;
        }
        else if (value === null)
        {
            // null
            if (_.isUndefined(type))
            {
                return null;
            }
            else if (isNativeList(type))
            {
                return [];
            }
            else if (type.Name == 'String')
            {
                return '';
            }
            else
            {
                return null;
            }
        }
        else if (typeof(value.GetType) == 'function')
        {
            if (isNativeList(value.GetType()))
            {
                // .Net List
                return convertNativeList(value)
            }
            else
            {
                // .Net object
                return convertNativeObject(value);
            }
        }
        else if (_.isArray(value))
        {
            // JS array
            return convertArray(value);
        }
        else
        {
            // JS object
            return convertObject(value);
        }
    }


    th.ConvertToJson = function (data)
    {
        return convertItem(data);
    };

})();
