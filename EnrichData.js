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

generateServiceShortName = function(name)
{
    name = namespace.normalize(name).toLowerCase();
    if (name.endsWith('service'))
        name = name.substr(0, name.length - 7).trim();

    if (!_.isUndefined(information.shortNames[name]))
    {
        name = information.shortNames[name];
    }
    else
    {
        name += ' service';
    }
    return namespace('{files}').fileName('ble', name);
}


function findCharacteristic(service, type)
{
    type = type.trim();

    for (var i in service.AllCharacteristics)
    {
        var characteristic = service.AllCharacteristics[i];
        if (characteristic.Type.trim() == type)
            return i;
    }

    return false;
}


function resolveCharacteristicDependencies(service)
{
    // Field containing all characteristics (also nested)
    service.AllCharacteristics = [];

    // Add all root characteristics
    for (var i in service.Characteristics)
    {
        service.AllCharacteristics.push(service.Characteristics[i]);
    }

    // Contains references: referenceMap[characteristic_index] = [referenced_characteristic_index, ...]
    var referenceMap = [];

    // Add nested characteristics
    for (var i = 0; i < service.AllCharacteristics.length; i++)
    {
        var characteristic = service.AllCharacteristics[i];
        referenceMap[i] = [];
        for (var k in characteristic.Fields)
        {
            var field = characteristic.Fields[k];
            if (!field.Format && field.Reference && field.Reference.trim())
            {
                // Field with nested characteristics detected 
                field.IsNested = true;
                var ref = field.Reference.trim();
                var nested = findCharacteristic(service, ref);
                if (nested !== false)
                {
                    // Required characteristic was already added
                    referenceMap[i].push(nested);
                }
                else if (!_.isUndefined(characteristicsDatabase[ref]))
                {
                    // Required characteristic is in database, so add it to this service
                    referenceMap[i].push(service.AllCharacteristics.length);
                    service.AllCharacteristics.push(JSON.parse(JSON.stringify(characteristicsDatabase[ref])));
                }
                else
                {
                    // Required characteristic cannot be resolved
                    log('WARNING: Cannot resolve characteristic reference "' + ref + '"');
                }
            }
            else
            {
                field.IsNested = false;
            }
        }
    }

    // Reorder characteristic, so referenced characteristic is before characteristic that references it
    var reordered = [];
    var pushed = [];

    // Function pushes characteristic from 'index' to 'reordered' array only if it is not already there
    function pushReordered(index)
    {
        if (_.isUndefined(pushed[index]))
        {
            pushed[index] = 1;
            for (var i in referenceMap[index])
            {
                pushReordered(referenceMap[index][i]);
            }
            pushed[index] = 2;
            reordered.push(service.AllCharacteristics[index]);
        }
        else if (pushed[index] == 1)
        {
            log('WARNING: Circular characteristic references.');
        }
    }

    for (var i in service.AllCharacteristics)
    {
        pushReordered(i);
    }

    service.AllCharacteristics = reordered;

    // Generate NestedIndex for each field
    for (var i = 0; i < service.AllCharacteristics.length; i++)
    {
        var characteristic = service.AllCharacteristics[i];
        for (var k in characteristic.Fields)
        {
            var field = characteristic.Fields[k];
            if (field.IsNested)
            {
                field.NestedIndex = findCharacteristic(service, field.Reference);
            }
            else
            {
                field.NestedIndex = false;
            }
        }
    }
}


function enrichEnumerationRanges(profile, service, characteristic, itemName, enumerations, ranges, type)
{
    _.each(ranges, function(item)
    {
        if (item.Name == '')
        {
            item.DefineMinName = namespace(false).defineName(characteristic.UniqueName, itemName, type, 'MIN');
            item.DefineMaxName = namespace(false).defineName(characteristic.UniqueName, itemName, type, 'MAX');
        }
        else
        {
            item.DefineMinName = namespace().defineName(characteristic.UniqueName, itemName, item.Name, 'MIN');
            item.DefineMaxName = namespace().defineName(characteristic.UniqueName, itemName, item.Name, 'MAX');
        }
        item.Type = type;
        enumerations.AllRanges.push(item);
    });
}


function enrichEnumerationItem(profile, service, characteristic, fieldName, bit, item)
{
    item.DefineName = namespace().defineName(characteristic.UniqueName, fieldName, bit ? bit.Name : '', item.Value);

    if (item.Requires && item.Requires != 'NA')
    {
        var name = item.Requires.toLowerCase();
        if (_.isUndefined(characteristic.AllConditions[name]))
            characteristic.AllConditions[name] = [];
        if (bit)
        {
            characteristic.AllConditions[name].push({ type : 'bit', field : fieldName, mask : bit.DefineMaskName, value : item.DefineName });
        }
        else
        {
            characteristic.AllConditions[name].push({ type : 'enum', field : fieldName, value : item.DefineName });
        }
    }
}


function enrichEnumerations(profile, service, characteristic, fieldName, bit, enumerations)
{
    _.each(enumerations.Enumeration, function(item)
    {
        enrichEnumerationItem(profile, service, characteristic, fieldName, bit, item)
    });

    enumerations.AllRanges = [];

    var fullName = namespace.fieldName(fieldName, bit ? bit.Name : '');

    enrichEnumerationRanges(profile, service, characteristic, fullName, enumerations, enumerations.EnumerationRange, '');
    enrichEnumerationRanges(profile, service, characteristic, fullName, enumerations, enumerations.ReservedForFutureUse, 'Reserved For Future Use');
    enrichEnumerationRanges(profile, service, characteristic, fullName, enumerations, enumerations.ReservedForFutureUse1, 'Reserved For Future Use');
    enrichEnumerationRanges(profile, service, characteristic, fullName, enumerations, enumerations.VendorSpecific, 'Vendor Specific');
    enrichEnumerationRanges(profile, service, characteristic, fullName, enumerations, enumerations.DefinedByServiceSpecification, 'Defined By Service Specification');
    enrichEnumerationRanges(profile, service, characteristic, fullName, enumerations, enumerations.Reserved, 'Reserved');

    enumerations.IsEmpty = ((enumerations.Enumeration.length + enumerations.AllRanges.length) == 0);

    return true;
}


function enrichAdditionalValues(profile, service, characteristic, field, additionalValues)
{
    _.each(additionalValues.Enumeration, function(item)
    {
        enrichEnumerationItem(profile, service, characteristic, field.UniqueName, '', item);
    });

    additionalValues.AllRanges = [];

    enrichEnumerationRanges(profile, service, characteristic, field, additionalValues, additionalValues.ReservedForFutureUse, 'Reserved For Future Use');

    additionalValues.IsEmpty = ((additionalValues.Enumeration.length + additionalValues.AllRanges.length) == 0);

    return true;
}


function enrichBitField(profile, service, characteristic, field, bitField)
{
    _.each(bitField.Bits, function(bit)
    {
        if (bit.Enumerations)
        {
            bit.DefineMaskName = namespace().defineName(characteristic.UniqueName, field.UniqueName, bit.Name, 'MASK');
            bit.DefineIndexName = namespace().defineName(characteristic.UniqueName, field.UniqueName, bit.Name, 'INDEX');
            bit.DefineSizeName = namespace().defineName(characteristic.UniqueName, field.UniqueName, bit.Name, 'SIZE');
            enrichEnumerations(profile, service, characteristic, field.UniqueName, bit, bit.Enumerations);
        }
    });
}


function enrichField(profile, service, characteristic, field)
{
    field.UniqueName = namespace(characteristic.StructName).fieldName(field.Name);
    field.UnitName = field.Unit.trim();
    if (field.UnitName)
    {
        if (!_.isUndefined(information.units[field.UnitName]))
            field.UnitName = information.units[field.UnitName].name;
        else if (!_.isUndefined(information.units[field.UnitName.replace('meter', 'metre')])) // Workaround for wrong unit spelling in BDS
            field.UnitName = information.units[field.UnitName.replace('meter', 'metre')].name;
    }

    if (field.Format)
    {
        if (!_.isUndefined(information.formatInformation[field.Format]))
        {
            field.FormatInformation = information.formatInformation[field.Format];
        }
        else
        {
            var ctype = namespace.structName('ble_type', field.Format, 't');
            field.FormatInformation = { ctype : ctype, carr : '', handler : namespace.structName('ble', field.Format), byValue: false, default: '' };
            log('WARNING: Format "' + field.Format + '" is not supported. Code for handling type "' + ctype + '" will NOT be generated.');
            profile.jobsForUser += 'YOUR_JOB: Format "' + field.Format + '" is not supported. You have to write your own implementation for "' + ctype + '" structure.\r\n';
        }
    }
    else if (field.IsNested)
    {
        var nested = findCharacteristic(service, field.Reference);
        if (nested !== false)
        {
            nested = service.AllCharacteristics[nested];
            field.FormatInformation = { ctype: nested.StructName, carr: '', handler: nested.ShortName, byValue: false, default: '' };
        }
        else
        {
            var ctype = namespace.structName(field.Reference);
            field.FormatInformation = { ctype : ctype, carr : '', handler : ctype, byValue: false, default: '' };
            log('WARNING: Format "' + field.Reference + '" is not supported. Code for handling type "' + ctype + '" will NOT be generated.');
            profile.jobsForUser += 'YOUR_JOB: Format "' + field.Format + '" is not supported. You have to write your own implementation for "' + ctype + '" structure.\r\n';
        }
    }
    else
    {
        field.FormatInformation = { ctype : '__unknown_type', carr : '', handler : '__unknown_type', byValue: false, default: '' };
        log('WARNING: Field "' + field.Name + '" have no format.');
        profile.jobsForUser += 'YOUR_JOB: Field "' + field.Name + '" in "' + characteristic.Name + '" have no format. Set it in BDS.';
    }

    if (field.Repeated)
    {
        field.RepeatedMaxDefine = namespace().defineName(characteristic.UniqueName, field.Name, 'MAX_ITEMS');
        field.RepeatedCounterName = namespace(characteristic.StructName).fieldName(field.Name, 'count');
    }

    if (field.FormatInformation.carr == '*' || field.FormatInformation.carr == '$')
    {
        field.IsArray = true;
        field.ArrayMaxDefine = namespace().defineName(characteristic.UniqueName, field.Name, 'MAX_LENGTH');
        if (field.FormatInformation.carr == '*')
        {
            field.ArrayLengthName = namespace(characteristic.StructName).fieldName(field.Name, 'length');
        }
    }

    if (field.Enumerations)
    {
        enrichEnumerations(profile, service, characteristic, field.UniqueName, null, field.Enumerations);
    }

    if (field.AdditionalValues)
    {
        enrichAdditionalValues(profile, service, characteristic, field, field.AdditionalValues);
    }

    if (field.BitField)
    {
        enrichBitField(profile, service, characteristic, field, field.BitField);
    }

    var and = [];

    for (var i in field.Requirement)
    {
        var req = field.Requirement[i].toLowerCase();
        if (req == 'mandatory')
            continue; // nothing to do

        var match = (/^(c[0-9]+)(or(c[0-9]+))?(or(c[0-9]+))?(or(c[0-9]+))?$/).exec(req);

        if (match !== null)
        {
            var or = [];
            for (var k = 1; k <= 15; k += 2)
            {
                if (!match[k]) break;
                var subName = match[k];
                if (_.isUndefined(characteristic.AllConditions[subName]))
                    characteristic.AllConditions[subName] = null; // will be assigned later
                or.push('{{{' + subName + '}}}');
            }
            and.push(or.length == 1 ? or[0] : '(' + or.join(') || (') + ')');
        }
        else
        {
            if (req != 'optional' && req != 'conditional')
            {
                log('WARNING: Unknown Requirement "' + field.Requirement[i] + '" in ' + characteristic.Name + ' -> ' + field.Name);
            }
            var name = namespace(characteristic.StructName).fieldName(field.UniqueName, 'is_present');
            and.push('{{{' + name + '}}}');
            characteristic.AllConditions[name] = [{ type : 'field', forField : field.UniqueName }];
        }
    }

    field.RequirementExpression = (and.length == 0 ? '' : (and.length == 1 ? and[0] : '(' + and.join(') && (') + ')'));

    profile.options[field.FormatInformation.handler] = true;
}


function enrichCharacteristic(profile, service, characteristic)
{
    characteristic.UniqueName = namespace('{characteristics}').structName(service.UniqueName, characteristic.Name);
    characteristic.StructName = namespace().structName(characteristic.UniqueName, 't');
    characteristic.ShortName = namespace('{static}', characteristic.UniqueName).functionName(characteristic.Name);
    characteristic.EncodeName = characteristic.ShortName + '_encode';
    characteristic.DecodeName = characteristic.ShortName + '_decode';
    characteristic.AllConditions = {};
    characteristic.Writable = false;
    characteristic.DecodeRequired = false;
    characteristic.EncodeRequired = false;
    characteristic.DecodeNestedRequired = false;

    for (var i in characteristic.Properties)
    {
        var prop = characteristic.Properties[i];
        if (prop.Write != 'Excluded' || prop.WriteWithoutResponse != 'Excluded' || prop.SignedWrite != 'Excluded' || prop.ReliableWrite != 'Excluded')
        {
            characteristic.Writable = true;
        }
    }

    for (var i in characteristic.Fields)
    {
        enrichField(profile, service, characteristic, characteristic.Fields[i]);
    }

    for (var i in characteristic.AllConditions)
    {
        if (characteristic.AllConditions[i] == null)
        {
            var name = namespace(characteristic.StructName).fieldName(i);
            log('WARNING: Requirement "' + i + '" in ' + characteristic.Name + ' is not defined. Adding field "bool ' + name + '".');
            characteristic.AllConditions[i] = [{ type : 'field', forField : 'unknown' }];
        }
    }

    var optionsStructRequired = false;

    _.each(characteristic.AllConditions, function(condition)
    {
        if (condition.type == 'field')
            optionsStructRequired = true;
    });

    _.each(characteristic.Fields, function(field)
    {
        if (field.Repeated || (field.IsArray && field.ArrayLengthName))
            optionsStructRequired = true;
        if (field.IsNested)
        {
            var nested = findCharacteristic(service, field.Reference);
            if (nested !== false && service.AllCharacteristics[nested].OptionsStructName)
                optionsStructRequired = true;
        }
    });

    if (optionsStructRequired)
    {
        characteristic.OptionsStructName = namespace().structName(characteristic.UniqueName, 'options_t');
    }
}


function resolveParsersDependencies(profile, service, characteristic)
{
    for (var i in characteristic.Fields)
    {
        var field = characteristic.Fields[i];
        if (field.IsNested)
        {
            var nested = findCharacteristic(service, field.Reference);
            if (nested !== false)
            {
                nested = service.AllCharacteristics[nested];
                nested.DecodeNestedRequired = true;
                resolveParsersDependencies(profile, service, nested);
            }
        }
    }
}


function enrichService(profile, service)
{
    service.UniqueName = generateServiceShortName(service.Name);
    service.HeaderFile = service.UniqueName + '.h';
    service.SourceFile = service.UniqueName + '.c';

    for (var i in service.ErrorCodes)
    {
        service.ErrorCodes[i].UniqueName = namespace().defineName(service.UniqueName, 'ERROR', service.ErrorCodes[i].Name);
    }

    resolveCharacteristicDependencies(service);

    for (var i in service.AllCharacteristics)
    {
        enrichCharacteristic(profile, service, service.AllCharacteristics[i]);
    }

    for (var i in service.Characteristics)
    {
        service.Characteristics[i].EncodeRequired = true;
        if (service.Characteristics[i].Writable)
        {
            service.Characteristics[i].DecodeRequired = true;
            service.Characteristics[i].DecodeNestedRequired = true;
            resolveParsersDependencies(profile, service, service.Characteristics[i]);
        }
    }
}


enrichProfile = function enrichProfile(profile)
{
    profile.options = {};
    profile.jobsForUser = '';

    _.each(profile.Services, function(service)
    {
        enrichService(profile, service);
    });

    // resolve all information.dependencies
    do
    {
        var allResolved = true;
        _.each(profile.options, function(val, name)
        {
            if (val && information.dependencies[name])
            {
                _.each(information.dependencies[name], function(dep)
                {
                    allResolved = (allResolved && profile.options[dep]);
                    profile.options[dep] = true;
                });
            }
        });
    } while (!allResolved);

    for (var i in profile.UniqueUUIDItems)
    {
        if (profile.UniqueUUIDItems[i].UUID.length > 4)
        {
            profile.jobsForUser += 'YOUR_JOB: If your profile uses some 128-bit UUIDs change NRF_SDH_BLE_VS_UUID_COUNT in "sdk_config.h" file\r\n' +
                                   'and adjust memory start address and size in project files.\r\n';
            break;
        }
    }
}
