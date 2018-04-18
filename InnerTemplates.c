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

=============================================================== CHAR_STRUCT_DECL ===============================================================


---
--- <% /*============================================== Defines for maximum string length, array size and repeat count ==============================================*/ %>
---

<% _.each(AllCharacteristics, function(characteristic) { %>
    <% _.each(characteristic.Fields, function(field) { %>
        <% if (field.Repeated) { %>
            ---
            /* YOUR_JOB: Define maximum repeat count of field <%= field.UniqueName %> */
            #ifndef <%= field.RepeatedMaxDefine %>
            #define <%= field.RepeatedMaxDefine %> <%= information.repeatedCountMaxDefine %>
            #endif
        <% } %>
        <% if (field.IsArray) { %>
            ---
            /* YOUR_JOB: Define maximum array length of field <%= field.UniqueName %> */
            #ifndef <%= field.ArrayMaxDefine %>
            #define <%= field.ArrayMaxDefine %> <%= information.arrayLengthMaxDefine %>
            #endif
        <% } %>
    <% }); %>
<% }); %>

---
--- <% /*============================================== Characteristics Fields Enumerations And Values ==============================================*/ %>
---

<%
function showEnumerations(obj, shift, text)
{
    if (obj && !obj.IsEmpty)
    {
        %>
        /* <%= text %> */
        <% _.each(obj.Enumeration, function(item) { %>
            #define <%= item.DefineName %> <%= shift ? '(' + item.Key + ' << ' + shift + ')': item.Key %> <%
                if (item.Description || (item.Requires && item.Requires != 'NA'))
                {
                    print('/**< ');
                    if (item.Requires && item.Requires != 'NA')
                        print('Requires: ' + item.Requires + '. ');
                    print(util.singleLine(item.Description) + ' */');
                }
                %>
        <% }); %>
        <% _.each(obj.AllRanges, function(item) { %>
            #define <%= item.DefineMinName %> <%= item.Start %> <%= (item.Condition || item.Type) ? '/**< ' + util.singleLine(item.Condition + ' ' + item.Type) + ' */' : '' %>
            #define <%= item.DefineMaxName %> <%= item.End   %> <%= (item.Condition || item.Type) ? '/**< ' + util.singleLine(item.Condition + ' ' + item.Type) + ' */' : '' %>
        <% }); %>
        ---
        <%
    };
}
%>

<% _.each(AllCharacteristics, function(characteristic) { %>
    <% _.each(characteristic.Fields, function(field) { %>

        <% showEnumerations(field.AdditionalValues, 0, 'Additional values for ' + characteristic.StructName + '::' + field.UniqueName); %>
        <% showEnumerations(field.Enumerations, 0, 'Enumeration values for ' + characteristic.StructName + '::' + field.UniqueName); %>

        <% if (field.BitField && field.BitField.Bits.length) { %>
            /* Bit handling for <%= characteristic.StructName %>::<%= field.UniqueName %> */
            <% _.each(field.BitField.Bits, function(bit) { %>
                #define <%= bit.DefineMaskName  %> (((1u << <%= bit.Size  %>) - 1u) << <%= bit.Index %>)
                #define <%= bit.DefineIndexName %> <%= bit.Index %>
                #define <%= bit.DefineSizeName  %> <%= bit.Size  %>
            <% }); %>
            ---
            <% _.each(field.BitField.Bits, function(bit) { %>
                <% showEnumerations(bit.Enumerations, 1 * bit.Index, 'Enumeration values for bits "' + bit.Name + '" in ' + characteristic.StructName + '::' + field.UniqueName); %>
            <% }); %>
        <% }; %>

    <% }); %>
<% }); %>

---
--- <% /*============================================== Characteristics Structure Definitions ==============================================*/ %>
---

<% _.each(AllCharacteristics, function(characteristic) { %>
    /** @brief <%= characteristic.Name %>.
     * ---
     * General information:
     * --- - UUID:        <%= util.uuidAsString(characteristic.UUID) %>
     * --- - Type:        <%= characteristic.Type %>
     * --- - Requirement: <%= characteristic.Requirement %>
     * ---
     * <%= characteristic.InformativeText.Abstract %>
     * ---
     * <%= characteristic.InformativeText.Summary %>
     * ---
     * <%= characteristic.InformativeText.InformativeDisclaimer %>
     * ---
     * <% if (characteristic.InformativeText.Examples.length) { print("Examples:"); _.each(characteristic.InformativeText.Examples, function(example) { %>
     *     --- - <%= example %><% }); } %>
     */
    typedef struct
    {
        <% _.each(characteristic.Fields, function(field) { %>
            /** @brief <%= field.Name %>.
             * ---
             * <%= field.Description %>
             * - Requirement:  <%= field.Requirement.join(' ') %>
             * <%= field.UnitName != ''        ? '- Unit:         ' + field.UnitName        : '' %>
             * <%= field.Format != ''          ? '- Format:       ' + field.Format          : '' %>
             * <%= field.DecimalExponent != '' ? '- Decimal Exp.: ' + field.DecimalExponent : '' %>
             * <%= field.BinaryExponent != ''  ? '- Binary Exp.:  ' + field.BinaryExponent  : '' %>
             * <%= field.Multiplier != ''      ? '- Multiplier:   ' + field.Multiplier      : '' %>
             * <%= field.Minimum != ''         ? '- Minimum:      ' + field.Minimum         : '' %>
             * <%= field.Maximum != ''         ? '- Maximum:      ' + field.Maximum         : '' %>
             * <% if (field.BitField && field.BitField.Bits.length > 0) { %>
             *     - Bits:
             *     <% _.each(field.BitField.Bits, function(bit) { %>
             *         --- - <%= bit.Size <= 1 ? bit.Index : bit.Index + '-' + (1 * bit.Index + 1 * bit.Size - 1) %> - <%= bit.Name %>
             *     <% }) %>
             * <% } %>
             * ---
             * <% if ((field.Enumerations && !field.Enumerations.IsEmpty) || (field.AdditionalValues && !field.AdditionalValues.IsEmpty)) { %>
             * See defined values <%= namespace.defineName(characteristic.UniqueName, field.UniqueName) %>_*
             * <% } %>
             */
            <% if (!field.IsArray) { %>
                <% if (!field.Repeated) { %>
                    <%= field.FormatInformation.ctype %> <%= field.UniqueName %>;
                <% } else { %>
                    <%= field.FormatInformation.ctype %> <%= field.UniqueName %>[<%= field.RepeatedMaxDefine %>];
                    ---
                    /** @brief Number of elements in field "<%= field.UniqueName %>".
                     */
                    uint16_t <%= field.RepeatedCounterName %>;
                <% } %>
            <% } else { %>
                <% if (!field.Repeated) { %>
                    <%= field.FormatInformation.ctype %> <%= field.UniqueName %>[<%= field.ArrayMaxDefine + (field.ArrayLengthName ? '' : ' + 1') %>];
                    <% if (field.ArrayLengthName) { %>
                        ---
                        /** @brief Length of field "<%= field.UniqueName %>".
                         */
                        uint16_t <%= field.ArrayLengthName %>;
                    <% } %>
                <% } else { %>
                    <%= field.FormatInformation.ctype %> <%= field.UniqueName %>[<%= field.ArrayMaxDefine + (field.ArrayLengthName ? '' : ' + 1') %>][<%= field.RepeatedMaxDefine %>];
                    <% if (field.ArrayLengthName) { %>
                        ---
                        /** @brief Length of field "<%= field.UniqueName %>".
                         */
                        uint16_t <%= field.ArrayLengthName %>[<%= field.RepeatedMaxDefine %>];
                    <% } %>
                    ---
                    /** @brief Number of elements in field "<%= field.UniqueName %>".
                     */
                    uint16_t <%= field.RepeatedCounterName %>;
                <% } %>
            <% } %>
            ---
        <% }); %>

        <% _.each(characteristic.AllConditions, function(list, name) { _.each(list, function(cond) { if (cond.type == 'field') { %>
            /** @brief Flag that controls optional field "<%= characteristic.AllConditions[name].forField %>".
             */
            bool <%= name %>;
            ---
        <% }});}); %>

        <%_.each(characteristic.Descriptors, function(descriptor) { if(descriptor.UUID !== '2902' && descriptor.Fields.length > 0) { %>
            <%= namespace.structName(UniqueName, characteristic.Name, descriptor.Name, '_t') %> <%= namespace.fieldName(characteristic.Name, descriptor.Name) %>;
        <% }}); %>

        <% if (characteristic.Fields.length == 0) { %>
            int _empty; /* JOUR_JOB: This field was added, because characteristic "<%= characteristic.Name %>" has no fields. Add some fields in BDS or implement it manually. */
        <% }; %>

    } <%= characteristic.StructName %>;
    ---
<% }); %>



=============================================================== CHAR_PARSERS ===============================================================



<% _.each(AllCharacteristics, function(characteristic) { %>

    static void <%= characteristic.EncodeName %>_to_buffer(bds_char_enc_buffer_t * buffer, const <%= characteristic.StructName %> * data)
    {
        <% for (var i in characteristic.Fields)
        {
            if (characteristic.Fields[i].Repeated)
            {
                print("uint32_t i;");
                break;
            }
        } %>
        <% _.each(characteristic.Fields, function(field) { %>

            <% if (field.RequirementExpression) { %>
                if (<%
                    var exp = field.RequirementExpression;
                    for (var name in characteristic.AllConditions)
                    {
                        var part = [];
                        _.each(characteristic.AllConditions[name], function(cond)
                        {
                            switch (cond.type)
                            {
                                case 'field':
                                    part.push('data->' + name);
                                    break;
                                case 'enum':
                                    part.push(cond.value + ' == data->' + cond.field);
                                    break;
                                case 'bit':
                                    part.push(cond.value + ' == (data->' + cond.field + ' & ' + cond.mask + ')');
                                    break;
                            }
                        });
                        part = (part.length == 1 ? part[0] : '(' + part.join(') || (') + ')');
                        exp = exp.replace('{{{' + name + '}}}', part);
                    }
                    print(exp);
                %>)
                {
            <% } %>

            <% if (field.Repeated) { %>
                for (i = 0; i < data-><%= field.RepeatedCounterName %>; i++)
                {
            <% } %>

            <% if (!field.IsArray) { %>
                <% if (!field.Repeated) { %>
                    <%= field.FormatInformation.handler %>_encode_to_buffer(buffer, <%= field.FormatInformation.byValue ? '' : '&' %>data-><%= field.UniqueName %>);
                <% } else { %>
                    <%= field.FormatInformation.handler %>_encode_to_buffer(buffer, <%= field.FormatInformation.byValue ? '' : '&' %>data-><%= field.UniqueName %>[i]);
                <% } %>
            <% } else { %>
                <% if (!field.Repeated) { %>
                    <% if (!field.ArrayLengthName) { %>
                        <%= field.FormatInformation.handler %>_encode_to_buffer(buffer, data-><%= field.UniqueName %>, sizeof(data-><%= field.UniqueName %>) / sizeof(data-><%= field.UniqueName %>[0]));
                    <% } else { %>
                        <%= field.FormatInformation.handler %>_encode_to_buffer(buffer, data-><%= field.UniqueName %>, data-><%= field.ArrayLengthName %>);
                    <% } %>
                <% } else { %>
                    <% if (!field.ArrayLengthName) { %>
                        <%= field.FormatInformation.handler %>_encode_to_buffer(buffer, data-><%= field.UniqueName %>[i], sizeof(data-><%= field.UniqueName %>[i]) / sizeof(data-><%= field.UniqueName %>[i][0]));
                    <% } else { %>
                        <%= field.FormatInformation.handler %>_encode_to_buffer(buffer, data-><%= field.UniqueName %>[i], data-><%= field.ArrayLengthName %>[i]);
                    <% } %>
                <% } %>
            <% } %>

            <% if (field.Repeated) { %>
                }
            <% } %>

            <% if (field.RequirementExpression) { %>
                }
            <% } %>
        <% }); %>
    }

    ---

    <% if (characteristic.EncodeRequired) { %>

    static uint16_t <%= characteristic.EncodeName %>(const <%= characteristic.StructName %> * data, uint8_t * buffer, uint16_t buffer_size)
    {
        bds_char_enc_buffer_t enc_buffer;
        bds_char_enc_buffer_init(&enc_buffer, buffer, buffer_size);
        <%= characteristic.EncodeName %>_to_buffer(&enc_buffer, data);
        return bds_char_enc_buffer_done(&enc_buffer, buffer);
    }

    <% }; %>

    ---

		
	<% if (characteristic.DecodeNestedRequired) { %>

    static void <%= characteristic.DecodeName %>_from_buffer(bds_char_dec_buffer_t * buffer, <%= characteristic.StructName %> * data)
    {
        <% for (var i in characteristic.Fields)
        {
            if (characteristic.Fields[i].Repeated)
            {
                print("uint32_t i;");
                break;
            }
        } %>
        <% _.each(characteristic.Fields, function(field) { %>

            <% if (field.RequirementExpression) { %>
                if (<%
                    var exp = field.RequirementExpression;
                    for (var name in characteristic.AllConditions)
                    {
                        var part = [];
                        _.each(characteristic.AllConditions[name], function(cond)
                        {
                            switch (cond.type)
                            {
                                case 'field':
                                    part.push('data->' + name);
                                    break;
                                case 'enum':
                                    part.push(cond.value + ' == data->' + cond.field);
                                    break;
                                case 'bit':
                                    part.push(cond.value + ' == (data->' + cond.field + ' & ' + cond.mask + ')');
                                    break;
                            }
                        });
                        part = (part.length == 1 ? part[0] : '(' + part.join(') || (') + ')');
                        exp = exp.replace('{{{' + name + '}}}', part);
                    }
                    print(exp);
                %>)
                {
            <% } %>

            <% if (field.Repeated) { %>
                for (i = 0; i < data-><%= field.RepeatedCounterName %>; i++)
                {
            <% } %>

            <% if (!field.IsArray) { %>
                <% if (!field.Repeated) { %>
                    <% if (field.FormatInformation.byValue) { %>
                        data-><%= field.UniqueName %> = <%= field.FormatInformation.handler %>_decode_from_buffer(buffer);
                    <% } else { %>
                        <%= field.FormatInformation.handler %>_decode_from_buffer(buffer, &data-><%= field.UniqueName %>);
                    <% } %>
                <% } else { %>
                    <% if (field.FormatInformation.byValue) { %>
                        data-><%= field.UniqueName %>[i] = <%= field.FormatInformation.handler %>_decode_from_buffer(buffer);
                    <% } else { %>
                        <%= field.FormatInformation.handler %>_decode_from_buffer(buffer, &data-><%= field.UniqueName %>[i]);
                    <% } %>
                <% } %>
            <% } else { %>
                <% if (!field.Repeated) { %>
                    <% if (!field.ArrayLengthName) { %>
                        <%= field.FormatInformation.handler %>_decode_from_buffer(buffer, data-><%= field.UniqueName %>, sizeof(data-><%= field.UniqueName %>) / sizeof(data-><%= field.UniqueName %>[0]));
                    <% } else { %>
                        <%= field.FormatInformation.handler %>_decode_from_buffer(buffer, data-><%= field.UniqueName %>, data-><%= field.ArrayLengthName %>);
                    <% } %>
                <% } else { %>
                    <% if (!field.ArrayLengthName) { %>
                        <%= field.FormatInformation.handler %>_decode_from_buffer(buffer, data-><%= field.UniqueName %>[i], sizeof(data-><%= field.UniqueName %>[i]) / sizeof(data-><%= field.UniqueName %>[i][0]));
                    <% } else { %>
                        <%= field.FormatInformation.handler %>_decode_from_buffer(buffer, data-><%= field.UniqueName %>[i], data-><%= field.ArrayLengthName %>[i]);
                    <% } %>
                <% } %>
            <% } %>

            <% if (field.Repeated) { %>
                }
            <% } %>

            <% if (field.RequirementExpression) { %>
                }
            <% } %>
        <% }); %>
    }

	<% }; %>

    ---

	<% if (characteristic.DecodeRequired) { %>

    static bool <%= characteristic.DecodeName %>(uint16_t buffer_size, const uint8_t * buffer, <%= characteristic.StructName %> * data)
    {
        bds_char_dec_buffer_t dec_buffer;
        bds_char_dec_buffer_init(&dec_buffer, buffer, buffer_size);
        <%= characteristic.DecodeName %>_from_buffer(&dec_buffer, data);
        return bds_char_dec_buffer_done(&dec_buffer, buffer);
    }

	<% }; %>

    ---

<% }); %>



=============================================================== CHAR_DEFAULTS ===============================================================

<%

var genDefaults = function(identifier, service, characteristic)
{
    _.each(characteristic.Fields, function(field)
    {
        if (field.IsNested && field.NestedIndex >= 0)
        {
            nested = service.AllCharacteristics[field.NestedIndex];
            if (!field.Repeated)
            { 
                genDefaults(identifier + '.' + field.UniqueName, service, nested);
            }
            else
            {
                genDefaults(identifier + '.' + field.UniqueName + '[0]', service, nested);
                print('/* YOUR_JOB: Repeat count of field ' + field.UniqueName + ' below. Remember to set default values for each repeated field above. */\r\n');
                print(identifier + '.' + field.RepeatedCounterName + ' = 1;\r\n');
            }
        }
        else if (!field.Repeated)
        {
            print('/* YOUR_JOB: Set default value of ' + field.UniqueName + ' below. */\r\n');
            print(field.FormatInformation.default.replace(/\$\$/g, identifier + '.' + field.UniqueName) + '\r\n');
        }
        else
        {
            print('/* YOUR_JOB: Set default value of each repeated filed ' + field.UniqueName + ' below. */\r\n');
            print(field.FormatInformation.default.replace(/\$\$/g, identifier + '.' + field.UniqueName + '[0]') + '\r\n');
            print('/* YOUR_JOB: Repeat count of field ' + field.UniqueName + ' below. */\r\n');
            print(identifier + '.' + field.RepeatedCounterName + ' = 1;\r\n');
        }
    });

    _.each(characteristic.AllConditions, function(list, name)
    {
        _.each(list, function(cond)
        {
            if (cond.type == 'field')
            { 
                print('/* YOUR_JOB: Set default value of below condition */\r\n');
                print(identifier + '.' + name + ' = false;\r\n');
            }
        });
    });
}

%>

---
<% _.each(Services, function (service) { if (service.Name == showService.Name && service.UUID == showService.UUID && service.ID == showService.ID) { %>
    <% _.each(service.Characteristics, function (characteristic) { if (characteristic.Name == showCharacteristic.Name && characteristic.UUID == showCharacteristic.UUID && characteristic.ID == showCharacteristic.ID) { %>
        <% genDefaults(showName, service, characteristic); %>
    <% }}); %>
<% }}); %>
