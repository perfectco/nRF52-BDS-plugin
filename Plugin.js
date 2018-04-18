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

var pluginInfo = {
    "Name":"Nordic Semiconductor nRF5 1.3.0 SDK 14.0",
    "Description":"Generates header and code files for nRF5x.",
    "Author":"Nordic Semiconductor",
    "Version":"1.3.0",
    "IsClient":false
};
var bdsVersion = "unknown";
var rawDataString = "{}";
vsUUIDCounter = 0;

function IncludeScript(filename) {
    var include = FileManager.ReadFile(filename);
    var newfunc = new Function(include);
    newfunc();
}

function IncludeJSON(filename) {
    var include = FileManager.ReadFile(filename);
    include = include.replace(/\s/g, ' ');
    var newfunc = new Function('return ' + include + ';');
    return newfunc();
}

function GetInfo(infoObject) {
    infoObject.Name = pluginInfo.Name;
    infoObject.Description = pluginInfo.Description;
    infoObject.Author = pluginInfo.Author;
    infoObject.Version = pluginInfo.Version;
    infoObject.IsClient = pluginInfo.IsClient;

    return infoObject;
}

function generateFiles(jsondata) {
    var data;
    pluginInfo.AppVersion = jsondata.AppVersion;
    jsondata.PluginInfo = pluginInfo;

    // Temporarily store service short names (used to check for uniqueness of service within profile)
    var serviceNames = [];
    _.each(jsondata.Services, function (service) {
        service.ShortName = getServiceShortName(service.Name);
        serviceNames.push(service.ShortName);
    });
    var uniqueServiceNames = _.countBy(serviceNames, function(name) { return name; });

    // Enrich services and generate header/code files
    _.each(jsondata.Services, function (service, serviceNumber) {
        service.IsUnique = (uniqueServiceNames[service.ShortName] === 1);
        enrich(service);
        service.enriched = jsondata.enriched.Services[serviceNumber];
        log("Generating files for service " + service.Name + "...");
        data = createHeaderFile(service);
        FileManager.CreateFile("ble_" + service.ShortName + ".h", data);
        data = createCodeFile(service);
        FileManager.CreateFile("ble_" + service.ShortName + ".c", data);
    });

    // Get unique service names (possibly with UUIDs)
    uniqueServiceNames = _.countBy(jsondata.Services, function(service) { return service.ShortName; });

    // Append counter value to differentiate variable names of identical services within profile, if needed
    for(var i = jsondata.Services.length - 1; i >= 0; i--) {
        var shortName = jsondata.Services[i].ShortName;
        if (uniqueServiceNames[shortName] > 1) {
            jsondata.Services[i].UniqueName = shortName + "_" + uniqueServiceNames[shortName];
        } else {
            jsondata.Services[i].UniqueName = shortName;
        }
        uniqueServiceNames[shortName]--;
    }

    // Generate template-to-generated-code interface files
    log("Generating SDK template project interface files...");
    data = createInterfaceHeaderFile();
    FileManager.CreateFile("service_if.h", data);

    data = createInterfaceCodeFile(jsondata);
    FileManager.CreateFile("service_if.c", data);
}

function RunPlugin(profiledata) {

    try {
        bdsVersion = profiledata.AppVersion.toString();
        log("BDS version: " + bdsVersion);

        converter = {};
        IncludeScript("json_converter.js");
        IncludeScript("underscore_customized.js");

        IncludeScript("Extensions.js");
        IncludeScript("Util.js");
        IncludeScript("Prettify.js");
        IncludeScript("Namespace.js");
        IncludeScript("EnrichData.js");

        information = IncludeJSON("information.json");
        characteristicsDatabase = IncludeJSON("CharacteristicsDatabase.json");

        innerTemplates = util.splitSections(FileManager.ReadFile("InnerTemplates.c"));
        for (var i in innerTemplates) {
            innerTemplates[i] = _.template(innerTemplates[i]);
        }

        log("Starting code generation...");

        if (!information.readDumpData) {

            if (_.isUndefined(profiledata.GetType)) {
                // already converted
                jsondata = profiledata;
            } else {
                jsondata = converter.ConvertToJson(profiledata);
            }

        } else {

            var dumpData = JSON.parse(FileManager.ReadFile('DataDump.json'));
            if (dumpData.bdsVersion != bdsVersion) {
                log("WARNING: Data dump was generated by different BDS version: current = " + bdsVersion + ", data = " + dumpData.bdsVersion);
            }
            if (dumpData.pluginInfo.Name != pluginInfo.Name || dumpData.pluginInfo.Version != pluginInfo.Version) {
                var message = "ERROR: Data dump was generated by different plugin version: current = " + pluginInfo.Name + " (" + pluginInfo.Version + "), data = " + dumpData.pluginInfo.Name + " (" + dumpData.pluginInfo.Version + ")";
                log(message);
                throw new Error(message);
            }
            if (typeof(dumpData.rawData) != "object" || typeof(dumpData.rawData.AppVersion) == "undefined") {
                var message = "ERROR: Data dump does not contain raw data. Probably exception occurred during or before raw data creation.";
                log(message);
                throw new Error(message);
            }
            jsondata = dumpData.rawData;

        }
        rawDataString = JSON.stringify(jsondata);

        removeDuplicateServices(jsondata);

        jsondata.enriched = JSON.parse(JSON.stringify(jsondata));

        enrichProfile(jsondata.enriched);

        generateFiles(jsondata);

        log("Generating file char_enc_dec.h");
        var compiled = _.template(FileManager.ReadFile('char_enc_dec.h'));
        FileManager.CreateFile("char_enc_dec.h", prettify(compiled(jsondata.enriched)));

        log("Generating file char_enc_dec.c");
        var compiled = _.template(FileManager.ReadFile('char_enc_dec.c'));
        FileManager.CreateFile("char_enc_dec.c", prettify(compiled(jsondata.enriched)));

        log("Generating file Readme.txt");
        var compiled = _.template(FileManager.ReadFile('Readme.txt'));
        FileManager.CreateFile("Readme.txt", compiled(jsondata.enriched));
        log("Code generation complete.");
        log("");
        log("Read \"Readme.txt\" file from output directory for more details.");

        if (information.dumpDataAlways) {
            DumpData();
        }

    } catch (ex) {
        DumpData(ex);
        throw ex;
    }
}

function DumpData(exception) {

    try {

        var dumpData = {};

        dumpData.pluginInfo = pluginInfo;
        dumpData.bdsVersion = bdsVersion;
        try {
            dumpData.message = exception.toString();
        } catch (ex) { /* ignore */ }
        try {
            dumpData.rawData = JSON.parse(rawDataString);
        } catch (ex) { /* ignore */ }
        try {
            dumpData.enrichedData = jsondata;
        } catch (ex) { /* ignore */ }

        FileManager.CreateFile("DataDump.json", JSON.stringify(dumpData));

        log("'DataDump.json' file was created in output directory. Please attach this file if you want to report a bug.");

    } catch (ex) {

        try {
            log("FATAL!!! Exception occurred during data dumping...");
            log(ex.toString());
        } catch (ex2) {
            /* nothing to do */
        }
    }
}

function removeDuplicateServices(jsondata) {

    var services = jsondata.Services;
    var uniqueServices = [];

    _.each(services, function(service, index) {
        var serviceObj = service.Name + "_" + service.UUID;

        if (_.contains(uniqueServices, serviceObj)) {
            delete services[index];
        } else {
            uniqueServices.push(serviceObj);
        }
    });

    var cleanedServicesArray = new Array();
    _.each(services, function(service) {
        if (service) {
            cleanedServicesArray.push(service);
        }
    });
    jsondata.Services = cleanedServicesArray;
}

function enrich(service) {
    var baseExtras = {
        NormalizedName: function () {
            return utils.getNormalizedName(this.Name);
        }
    };

    var characteristicsExtras = _.extend({}, baseExtras, {
        isReadSupported: function () {
            return (this.Properties[0].Read !== PropertyRequirementEnum.Excluded);
        },
        isReadMandatory: function () {
            return (this.Properties[0].Read === PropertyRequirementEnum.Mandatory);
        },
        isNotifySupported: function () {
            return (this.Properties[0].Notify !== PropertyRequirementEnum.Excluded);
        },
        isNotifyMandatory: function () {
            return (this.Properties[0].Notify === PropertyRequirementEnum.Mandatory);
        },
        isIndicateSupported: function () {
            return (this.Properties[0].Indicate !== PropertyRequirementEnum.Excluded);
        },
        isIndicateMandatory: function () {
            return (this.Properties[0].Indicate === PropertyRequirementEnum.Mandatory);
        },
        isWriteSupported: function () {
            return (this.Properties[0].Write !== PropertyRequirementEnum.Excluded);
        },
        isWriteMandatory: function () {
            return (this.Properties[0].Write === PropertyRequirementEnum.Mandatory);
        },
        isWriteWithoutResponseSupported: function () {
            return (this.Properties[0].WriteWithoutResponse !== PropertyRequirementEnum.Excluded);
        },
        isWriteWithoutResponseMandatory: function () {
            return (this.Properties[0].WriteWithoutResponse === PropertyRequirementEnum.Mandatory);
        },
        isReliableWriteSupported: function () {
            return (this.Properties[0].ReliableWrite !== PropertyRequirementEnum.Excluded);
        },
        isReliableWriteMandatory: function () {
            return (this.Properties[0].ReliableWrite === PropertyRequirementEnum.Mandatory);
        },
        SendFunction: function () {
            // Check if we need to generate a "send" function
            if (this.isNotifySupported() && this.isIndicateSupported()) {
                return [
                    {
                        Postfix: 'notify',
                        Type: 'BLE_GATT_HVX_NOTIFICATION'
                    },
                    {
                        Postfix: 'indicate',
                        Type: 'BLE_GATT_HVX_INDICATION'
                    }
                ];
            } else if (this.isNotifySupported() || this.isIndicateSupported()) {
                return [
                    {
                        Postfix: 'send',
                        Type: this.isNotifySupported() ? 'BLE_GATT_HVX_NOTIFICATION' : 'BLE_GATT_HVX_INDICATION'
                    }
                ];
            }
        },
        StructName: function () {
            return "ble_" + service.ShortName.toLowerCase() + "_" + this.NormalizedName() + "_t";
        },
        getFullUuid: function() {
            return utils.getUuidBase(this.UUID);
        },
        hasNibbles: function() {
            return _.some(this.Fields, function(field) {
                return field.FormatType() === 'nibble';
            });
        }
    });

    var descriptorExtras = _.extend({}, baseExtras, {
        isReadSupported: function () {
            return (this.Properties.Read !== PropertyRequirementEnum.Excluded);
        },
        isReadMandatory: function () {
            return (this.Properties.Read === PropertyRequirementEnum.Mandatory);
        },
        isWriteSupported: function () {
            return (this.Properties.Write !== PropertyRequirementEnum.Excluded);
        },
        isWriteMandatory: function () {
            return (this.Properties.Write === PropertyRequirementEnum.Mandatory);
        },
        StructName: function () {
            var structname;
            if (this.Fields.length > 0) {
                structname = "ble_" + service.ShortName.toLowerCase() + "_" + this.NormalizedName() + "_t";
            }
            return structname;
        },
        hasNibbles: function() {
            return _.some(this.Fields, function(field) {
                return field.FormatType() === 'nibble';
            });
        }
    });

    var enumList = [];
    var fieldExtras = _.extend({}, baseExtras, {
        getEnumList: function(){
            return (enumList);
        },
        isStruct: function (fieldName) {
            return ((this.Enumerations && this.getEnumerations(fieldName).length > 0) || (this.BitField && this.BitField.Bits.length > 0));
        },
        hasRequirement: function () {
            return !_.some(this.Requirement,function(element) {
                var liste = ["Mandatory", "Optional", "Conditional"];
                return _.contains(liste, element);
            });
        },
        FormatType: function() {
            return utils.getTypeDef(this.Format, this.Reference);
        },
        FormatTypeName: function() {
            return this.FormatType() + "_t";
        },
        isVariableLength: function() {
            var var_length_types = ['uint8_array'];
            return _.contains(var_length_types, this.FormatType());
        },
        isEnumValid: function(fieldName){
            if (enumList.length === 0){
                enumList.push(fieldName);
                return true;
            }
            if (_.contains(enumList, fieldName)){
                return false;
            }
            var newList = [fieldName];
            enumList.push(newList);
            return true;
        },
        getEnumerations: function() {
            if (!_.isNull(this.Enumerations)) {
                return utils.resolveEnumDuplicates(this.Enumerations.Enumeration);
            }
        }
    });


    var enumBitList = [];
    var bitfieldExtras = _.extend({}, baseExtras, {
        NormalizedName: function (propName) {
            if (!_.isUndefined(this.CachedNormalizedName)) return this.CachedNormalizedName;
            if (_.isUndefined(propName)) propName = '';
            if (typeof(_global_bitfieldNames) == 'undefined') _global_bitfieldNames = {};
            if (_.isUndefined(_global_bitfieldNames[propName])) _global_bitfieldNames[propName] = {};
            var index = '';
            var result = utils.getNormalizedName(this.Name);
            if (result == '') {
                result = '_';
                index = 1;
            }
            while (_global_bitfieldNames[propName][result + index]) {
                index = 1 * index + 1;
            }
            _global_bitfieldNames[propName][result + index] = true;
            this.CachedNormalizedName = result + index;
            return result + index;
        },
        getEnumList: function(){
            return (enumBitList);
        },
        isBitEnumValid: function(fieldName, bitfieldName){
            var enumName = fieldName + "_" + bitfieldName;
            if (enumBitList.length === 0){
                enumBitList.push(enumName);
                return true;
            }
            if (_.contains(enumBitList, enumName)){
                return false;
            }
            enumBitList.push(enumName);
            return true;
        },
        getEnumerations: function() {
            if (!_.isNull(this.Enumerations)) {
                return utils.resolveEnumDuplicates(this.Enumerations.Enumeration);
            }
        }
    });

    var enumerationExtras = _.extend({}, baseExtras, {
        NormalizedName: function () {
            return utils.getNormalizedName(this.Value);
        },
        hasRequires: function () {
            return !_.isUndefined(this.Requires) && this.Requires !== "NA";
        }
    });

    _.extend(service, baseExtras);
    if(!service.IsUnique) {
        service.ShortName = getServiceShortName(service.Name) + '_' + utils.getNormalizedName(utils.getShortUuid(service.UUID));
    }
    service.FullUuid = utils.getUuidBase(service.UUID);
    service.PluginInfo = pluginInfo;

    _.each(service.Characteristics, function (characteristic) {
        _.extend(characteristic, characteristicsExtras);

        var indexOfFirstMatch = -1;
        _.each(characteristic.Descriptors, function (descriptor) { // Descriptors
            _.extend(descriptor, descriptorExtras);

            // Find the first descriptor which matches below UUID, tag it and store its index in the Descriptors array
            if(indexOfFirstMatch < 0) { // No match yet
                var firstMatch = _.find(characteristic.Descriptors, function (descriptor) {
                    return (descriptor.UUID === "2901" || descriptor.UUID === "2904");
                });

                if(!_.isUndefined(firstMatch)) {
                    indexOfFirstMatch = _.indexOf(characteristic.Descriptors, firstMatch);
                    descriptor.DescriptorAddSpecial = true;
                    characteristic.HasDescriptorAddSpecial = true;
                }
            }

            _.each(descriptor.Fields, function (field) { // Structs
                _.extend(field, fieldExtras);
                if (!_.isNull(field.Enumerations)) {
                    _.each(field.Enumerations.Enumeration, function (enumVal) {
                        _.extend(enumVal, enumerationExtras);
                    });
                }

                if(!_.isNull(field.BitField)) {
                    _.each(field.BitField.Bits, function (bitField) { // Enums
                        if (_.isNull(bitField.Name)) { // Hack to avoid null ref exception because of descriptor 0x2902 bit with null Name
                            bitField.Name =  service.Name + "_" + field.Name + "_bit_" + bitField.Index;
                        }
                        _.extend(bitField, bitfieldExtras);

                        // Enum values/members
                        if (!_.isNull(bitField.Enumerations)) {
                            _.each(bitField.Enumerations.Enumeration, function (enumVal) {
                                _.extend(enumVal, enumerationExtras);
                            });
                        }
                    });
                }
            });
        });

        // If a "special" descriptor was found (and it's not already at index 0), move it
        // to the beginning of the Descriptors array so it can be added first in code template
        if(indexOfFirstMatch > 0) {
            var removedItem = characteristic.Descriptors.splice(indexOfFirstMatch, 1);
            characteristic.Descriptors.unshift(removedItem[0]);
        }

        _.each(characteristic.Fields, function (field) { // Structs
            _.extend(field, fieldExtras);
            if (!_.isNull(field.Enumerations)) {
                _.each(field.Enumerations.Enumeration, function (enumVal) {
                    _.extend(enumVal, enumerationExtras);
                });
            }

            if(!_.isNull(field.BitField)) {
                _.each(field.BitField.Bits, function (bitField) { // Enums
                    if (_.isNull(bitField.Name)) { // Hack to avoid null ref exception because of descriptor 0x2902 bit with null Name
                        bitField.Name =  service.Name + "_" + field.Name + "_bit_" + bitField.Index;
                    }
                    _.extend(bitField, bitfieldExtras);

                    // Enum values/members
                    if (!_.isNull(bitField.Enumerations)) {
                        _.each(bitField.Enumerations.Enumeration, function (enumVal) {
                            _.extend(enumVal, enumerationExtras);
                        });
                    }
                });
            }
        });
    });
}


/**
 * Creates header file for the provided service
 * @param {JSON service object} service
 * @return {string} headerFile
 */
function createHeaderFile(service) {
    log("Generating header file ble_" + service.ShortName + ".h");
    var template = FileManager.ReadFile('header_template.h');
    var compiled = _.template(template);
    var data = compiled(service);
    return data;
}

/**
 * Creates code file for the provided service
 * @param {JSON service object} service
 * @return {string} codeFile
 */
function createCodeFile(service) {
    log("Generating code file ble_" + service.ShortName + ".c");
    var template = FileManager.ReadFile('code_template.c');
    var compiled = _.template(template);
    var data = compiled(service);
    return data;
}

/**
 * Creates header file for template-to-generated-code interface (static file)
 * @return {string} headerFile
 */
function createInterfaceHeaderFile() {
    log("Generating header file service_if.h");
    var template = FileManager.ReadFile('service_if_template.h');
    var compiled = _.template(template);
    var data = compiled();
    return data;
}

/**
 * Creates code file for all services in the provided profile (template-to-generated-code interface)
 * @param {JSON profile object} profile
 * @return {string} codeFile
 */
function createInterfaceCodeFile(profile) {
    log("Generating code file service_if.c");
    var template = FileManager.ReadFile('service_if_template.c');
    var compiled = _.template(template);
    var data = compiled(profile);
    return data;
}

/**
 * Returns lowercase three/four-letter short name for BLE profiles
 * See https://www.bluetooth.org/en-us/specification/adopted-specifications
 * @param {string} serviceName
 * @return {string} shortName
 */
function getServiceShortName(name) {

    name = namespace.normalize(name).toLowerCase();
    if (name.endsWith('service'))
        name = name.substr(0, name.length - 7).trim();

    if (!_.isUndefined(information.shortNames[name])) {
        name = information.shortNames[name];
    } else {
        name += ' service';
    }
    return namespace.fileName(name);
}

utils = typeof utils !== 'undefined' ? utils : {};
(function (ns) {
    ns.getNormalizedName = function (name) {
        if (name.indexOf(3) === 0) {
            name = "Three" + name.substring(1); // Spesialtilfelle der navnet starter med et 3 tall, ingen characteristics starter med et annet tall.
        }
        return name.toLowerCase().replace(/#|-| |:|\.|'|,|\(|\)|\/|[\u007B-\uFFFF]/g, '_');
    };

    ns.getUuidAsArray = function (uuid) {
        var re = /([0-9a-f]{2})([0-9a-f]{2})([0-9a-f]{2})([0-9a-f]{2})-([0-9a-f]{2})([0-9a-f]{2})-([0-9a-f]{2})([0-9a-f]{2})-([0-9a-f]{2})([0-9a-f]{2})-([0-9a-f]{2})([0-9a-f]{2})([0-9a-f]{2})([0-9a-f]{2})([0-9a-f]{2})([0-9a-f]{2})/i;
        var arr = re.exec(uuid);
        if (_.isNull( !_.isNull(arr) ? arr.reverse() : null)){
            var re2 = /([0-9a-f]{2})([0-9a-f]{2})([0-9a-f]{2})([0-9a-f]{2})([0-9a-f]{2})([0-9a-f]{2})([0-9a-f]{2})([0-9a-f]{2})([0-9a-f]{2})([0-9a-f]{2})([0-9a-f]{2})([0-9a-f]{2})([0-9a-f]{2})([0-9a-f]{2})([0-9a-f]{2})([0-9a-f]{2})/i;
            var arr2 = re2.exec(uuid);
            return !_.isNull(arr2) ? arr2.reverse() : null;
        } else {
            return !_.isNull(arr) ? arr.reverse() : null;
        }
    };

    ns.getUuidBase = function(uuid) {
        var arr = ns.getUuidAsArray(uuid);
        if (_.isNull(arr)) {
            return null;
        } else {
            var uuidBase = '0x' + arr[0];
            for (var i = 1; i < 16; i++) {

                if (i === 12 || i === 13) {
                    uuidBase += ', 0x00';
                } else {
                    uuidBase += ', 0x' + arr[i];
                }
            }
            var str = '0x' + arr[13] + arr[12];
            return {'UuidBase': uuidBase, 'Uuid' : str};
        }
    };

    ns.getShortUuid = function(uuid) {
        if((uuid.length === 36) || ((uuid.length === 32))) {
            return uuid.substring(4,4);
        }
        return uuid;
    };

    ns.resolveEnumDuplicates = function(enumerations) {
        var uniqueServiceNames = _.countBy(enumerations, function (enumVal) { return enumVal.Value; });
        _.each(enumerations, function (enumVal) {
            // Check for identical enum member names, append Key value to name if needed
            if (uniqueServiceNames[enumVal.Value] > 1) {
                enumVal.Value += '_' + enumVal.Key;
            }
        });
        return enumerations;
    };

    ns.getTypeDef = function (name, ref) {
        if (name === '2bit') {
            return 'uint8_array';
        }
        if (name === '4bit') {
            return 'nibble';
        }
        if (name === 'Item8bit' || name === '8bit') {
            return 'uint8';
        }
        if (name === 'Item16bit' || name === '16bit') {
            return 'uint16';
        }
        if (name === '24bit') {
            return 'uint24';
        }
        if (name === '32bit') {
            return 'uint32';
        }
        if (name === 'uint8[]') {
            return 'uint8_array';
        }
        if (name === 'uint12' || name === 'uint48' || name === 'uint128') {
            return 'uint8_array';
        }
        if (name === 'sint12' || name === 'sint24' || name === 'sint48' || name === 'sint128') {
            return 'uint8_array';
        }
        if (name === 'sint8') {
            return 'int8';
        }
        if (name === 'sint16') {
            return 'int16';
        }
        if (name === 'sint32') {
            return 'int32';
        }
        if (name === 'sint64') {
            return 'int64';
        }
        if (name === 'FLOAT') {
            return 'float32';
        }
        if (name === 'duint16') {
            return 'uint8';
        }
        if (name === 'utf8s') {
            return 'ble_srv_utf8_str';
        }
        if (name === 'utf16s') {
            return 'uint8_array';
        }
        if (name === 'characteristic'   ||
            name === 'struct'           ||
            name === 'gatt_uuid'        ||
            name === 'variable') {
            return 'uint8_array';
        }
        if (name === 'reg-cert-data-list') {
            return 'regcertdatalist';
        }
        if (ref === 'org.bluetooth.characteristic.digital') {
            return 'uint8_array';
        }
        if (ref === 'org.bluetooth.characteristic.analog') {
            return 'uint16';
        }
        if (ref === 'org.bluetooth.characteristic.exact_time_256') {
            return 'exact_time_256';
        }
        if (ref === 'org.bluetooth.characteristic.temperature_type') {
            return 'temperature_type';
        }
        if (ref === 'org.bluetooth.characteristic.temperature_measurement') {
            return 'temperature_measurement';
        }
        if (ref && ref.indexOf('date_time') > -1) {
            return 'ble_date_time';
        }
        if (ref && ref.indexOf('blood_pressure_measurement') > -1) {
            return 'ble_bls_blood_pressure_measurement';
        }
        if (ref && ref.indexOf('alert_category_id') > -1) {
            return 'uint8';
        }
        if (ref && ref.indexOf('time_source') > -1) {
            return 'uint8';
        }
        if (ref &&
           (ref.indexOf('time_accuracy') > -1   ||
            ref.indexOf('time.day')      > -1   ||
            ref.indexOf('time.hour')     > -1   ||
            ref.indexOf('dst_offset')    > -1)) {
            return 'uint8';
        }
        if (ref && ref.indexOf('time_zone') > -1) {
            return 'int8';
        }
        return (name !== null) ? name.toLowerCase() : 'UNDEFINED_TYPE';
    };

}(utils));

// "Enums"

// Bluetooth.D3T.Model.PropertyRequirements
// Used by Characteristic.Properties.Read/Write/...
// BDS only gives numerical values for these.
var PropertyRequirementEnum = Object.freeze({
    "Mandatory": "Mandatory",
    "Optional": "Optional",
    "Excluded": "Excluded",
    "C1": "C1",
    "C2": "C2",
    "C3": "C3",
    "C4": "C4",
    "C5": "C5",
    "C6": "C6",
    "C7": "C7",
    "C8": "C8",
    "C9": "C9",
    "C10": "C10"
});

// Bluetooth.D3T.Model.requirement
// Used by Characteristic field "Requirement"
// BDS gives string value (name of enum element, not numerical value)
var RequirementEnum = Object.freeze({
    "Unset": -1,
    "Mandatory": "Mandatory",
    "Optional": "Optional",
    "Conditional": "Conditional",
    "C1": "C1",
    "C2": "C2",
    "C2orC3": "C2orC3",
    "C3": "C3",
    "C4": "C4",
    "C5": "C5",
    "C6": "C6",
    "C7": "C7",
    "C8": "C8",
    "C9": "C9",
    "C10": "C10",
    "C11": "C11",
    "C12": "C12",
    "C13": "C13",
    "C14": "C14",
    "C15": "C15",
    "C16": "C16",
    "C17": "C17",
    "C18": "C18",
    "C19": "C19",
    "C20": "C20"

});

// Exports
if (typeof module !== 'undefined') {
    module.exports.createHeaderFile = createHeaderFile;
    module.exports.createCodeFile = createCodeFile;
    module.exports.enrich = enrich;
    module.exports.generateFiles = generateFiles;
}
