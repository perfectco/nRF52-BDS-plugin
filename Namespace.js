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

/* Util for resolving conflicted identifier names.
 * Usage:
 *      namespace("namespace", ...).elementName("name_parts", ...)
 *      "namespace" - namespace where identifier will be located.
 *          Pass more parameters for nested namespaces.
 *          Pass no parameters for global namespace.
 *          Skip "()" to generate name without checking if it is already taken.
 *      elementName - function that creates name for specific name, e.g. structName
 *      "name_parts" - parts that will be concatenated to create identifier name
 * 
 * Examples:
 *      namespace().defineName("My Module", "OPTION") will return "MY_MODULE_OPTION"
 *      or "MY_MODULE_OPTION_2" if this name is already taken in global namespace, or "MY_MODULE_OPTION_3", e.t.c.
 *      Warning will be shown if name already taken.
 *      namespace("my_struct_t").fieldName("Field") will return "field"
 *      or "field_2" if this name is already taken in structure "my_struct_t", or "field_3", e.t.c.
 *      namespace.defineName("BLE", "My Configuration") will return always "BLE_MY_CONFIGURATION" and will not
 *      register new identifier name.
 */
namespace = function()
{
    var ns = {};
    var prefix = '';
    var showWarning = true;

    for (var i = 0; i < arguments.length; i++)
    {
        if (typeof(arguments[i]) == 'boolean')
        {
            showWarning = arguments[i];
        }
        else
        {
            prefix += '::' + arguments[i];
        }
    }

    if (arguments.length == 0)
        prefix = '::';

    if (_.isUndefined(namespace.all[prefix]))
    {
        namespace.all[prefix] = {};
    }

    for (var i in namespace)
    {
        if (typeof(namespace[i]) == 'function')
        {
            (function(baseFunction)
            {
                ns[i] = function()
                {
                    var result = baseFunction.apply(namespace, arguments);
                    if (_.isUndefined(namespace.all[prefix][result]))
                    {
                        namespace.all[prefix][result] = true;
                        return result;
                    }
                    else
                    {
                        for (var k = 2; k < 1000000000; k++)
                        {
                            var args = [];
                            for (var n = 0; n < arguments.length; n++)
                                args.push(arguments[n]);
                            args.push(k);
                            var newResult = baseFunction.apply(namespace, args);
                            if (_.isUndefined(namespace.all[prefix][newResult]))
                            {
                                namespace.all[prefix][newResult] = true;
                                if (showWarning) log('WARNING: Name conflict. "' + result + '" already defined in "' + prefix + '". Renaming to "' + newResult + '"');
                                return newResult;
                            }
                        }
                    }
                }
            })(namespace[i]);
        }
    }

    return ns;
}

namespace.all = {};

/*
 * Converts all arguments to normalized form:
 *  - underscores replaced by spaces
 *  - one space between words
 *  - word starts with upper case and rest is lower case
 * This form is very flexible for further transforming 
 */
namespace.normalize = function()
{
    var ret = '';
    for (var i = 0; i < arguments.length; i++)
        ret += ' ' + arguments[i];
    return ret
        .toLowerCase()
        .replace(/[\s_]+/g, ' ')
        .replace(/\b\w/g, function(ch) { return ch.toUpperCase() })
        .trim();
}

namespace.structName = function()
{
    return this.normalize.apply(this, arguments)
        .replace(/[^a-zA-Z0-9]/g, '_')
        .replace(/_+/g, '_')
        .replace(/^_/, '')
        .replace(/_$/, '')
        .replace(/^([0-9])/, '_$1')
        .toLowerCase();
}

namespace.fileName = namespace.structName;

namespace.fieldName = namespace.structName;

namespace.enumName = namespace.structName;

namespace.functionName = namespace.structName;

namespace.enumItemName = function()
{
    return this.normalize.apply(this, arguments)
        .replace(/[^a-zA-Z0-9]/g, '_')
        .replace(/_+/g, '_')
        .replace(/^_/, '')
        .replace(/_$/, '')
        .replace(/^([0-9])/, '_$1')
        .toUpperCase();
}

namespace.defineName = namespace.enumItemName;
