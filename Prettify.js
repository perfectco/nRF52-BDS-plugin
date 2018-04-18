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

/*
 * Makes C source code better looking.
 * It does following things:
 *  - Ignores all line indentations and creates new ones based on the line content.
 *  - Skips all empty lines.
 *  - One or more lines with only "---" are replaced by one empty line.
 *  - Single line comments "//" are not supported.
 *  - Comments are converted to a format with "*" at the beginning of each line (like this comment).
 *  - Comments are aligned to code.
 *  - Comments are word-wrapped for maximum of "information.commentWordWrapLength" or 120 if undefined.
 *  - Indentations in comments are skipped. To make an indentation in a comment, use the "---" string.
 *  - One or more lines in comment with only "---" are replaced by one empty line.
 *  - Converts all line endings to Windows line ending "\r\n".
 */
prettify = function prettify(source, indent)
{
    if (typeof (information) == 'undefined' || typeof (information.commentWordWrapLength) == 'undefined')
        information.commentWordWrapLength = 120;

    function prettyComment(text)
    {
        function wordwrap(output, indent, line)
        {
            var max = information.commentWordWrapLength - indent.length - 4;
            while (line.length > max)
            {
                var re = new RegExp('\\s+', 'g');
                var cutAt = 0;
                var cutLen = 0;
                do
                {
                    var match = re.exec(line);
                    if (match === null || match.index > max) break;
                    cutAt = match.index;
                    cutLen = match[0].length;
                } while (match !== null && match.index <= max);
                if (cutAt == 0) break;
                output.push(indent + line.substr(0, cutAt));
                line = line.substring(cutAt + cutLen, line.length);
            }
            output.push(indent + line);
        }

        var output = [];

        // remove comment begin and end
        text = text.substring(2, text.length - 2);
        var doxygen = text.startsWith('*');
        var firstChar = ' ';
        if (doxygen)
        {
            text = text.substring(1, text.length);
            if (text.startsWith('<'))
            {
                firstChar = '<';
                text = text.substring(1, text.length);
            }
        }

        var lines = text.split('\n');

        var multiline = (lines.length > 1);

        for (var lineNumber in lines)
        {
            var line = lines[lineNumber].replace(/[\x00-\x08\x0A-\x19]/, '').trim();

            if (line.startsWith('*'))
                line = line.substr(1, line.length - 1).trim();

            var indent = 0;
            while (line.startsWith('---'))
            {
                line = line.substr(3, line.length - 3).trim();
                indent++;
            }

            if (line == '' && indent == 0)
            {
                // skip empty line
            }
            else if (line == '')
            {
                // add up to one empty line
                emptyLines++;
                if (emptyLines < 2)
                    output.push('');
            }
            else
            {
                emptyLines = 0;
                if (multiline)
                    wordwrap(output, ' '.repeat(4 * indent), line);
                else
                    output.push(' '.repeat(4 * indent) + line);
            }
        }

        while (output.length > 0 && output[output.length - 1] == '')
        {
            output.pop();
        }

        if (multiline)
        {
            return '/*' + (doxygen > 0 ? '*' : ' ') + firstChar + output.join('\r\n *  ') + '\r\n */';
        }
        else
        {
            if (firstChar == '<')
                firstChar = '< ';
            return '/*' + (doxygen > 0 ? '*' : '') + firstChar + output.join('\r\n *  ') + ' */';
        }
    }

    if (typeof(indent) == 'undefined') indent = 0;
    var continueLine = false;
    var inComment = false;

    var output = [];
    var emptyLines = 0;
    var indentDelta = 0;
    var indentMin = 0;
    var lastCodeChar = '';
    var firstCodeChar = '';

    function ind(indent, continueLine)
    {
        return ' '.repeat(4 * indent + (continueLine ? 8 : 0));
    }

    function parseLine(line)
    {
        indentDelta = 0;
        indentMin = 0;
        lastCodeChar = '';
        firstCodeChar = '';

        for (var i = 0; i < line.length; i++)
        {
            var char = line.charAt(i);
            if (inComment)
            {
                if (char == '*' && i + 1 < line.length && line.charAt(i + 1) == '/')
                {
                    // exit from comment
                    i++;
                    inComment = false;
                }
            }
            else
            {
                if (char == '/' && (i + 1 < line.length && line.charAt(i + 1) == '*'))
                {
                    // start comment
                    i++;
                    inComment = true;
                }
                else if (char.charCodeAt(0) > 32)
                {
                    lastCodeChar = char;
                    if (firstCodeChar == '')
                        firstCodeChar = char;
                    if (char == '{')
                    {
                        // increase indent on block start
                        indentDelta++;
                    }
                    else if (char == '}')
                    {
                        // decrease indent on block end and save minimum indent in this line
                        indentDelta--;
                        indentMin = Math.min(indentMin, indentDelta);
                    }
                }
            }
        }

        if (lastCodeChar != '')
        {
            if (lastCodeChar != '}' && lastCodeChar != '{' && lastCodeChar != ';' && firstCodeChar != '#')
            {
                continueLine = true;
            }
            else
            {
                continueLine = false;
            }
        }
    }


    var lines = source.replace(/\/\*[\s\S]*?\*\//g, prettyComment).split('\n');

    for (var lineNumber in lines)
    {
        var line = lines[lineNumber].replace(/[\x00-\x08\x0A-\x19]/, '').trim();

        if (inComment)
        {
            output.push(ind(indent, continueLine) + (line.startsWith('*') ? ' ' : '') + line);
            parseLine(line);
        }
        else
        {
            if (line == '')
            {
                // skip line
            }
            else if (line == '---')
            {
                emptyLines++;
                if (emptyLines < 2)
                    output.push('');
            }
            else
            {
                emptyLines = 0;
                var cnt = continueLine;
                parseLine(line);
                output.push(ind(indent + indentMin, cnt && firstCodeChar != '{') + line);
                indent += indentDelta;
            }
        }
    }

    return output.join('\r\n') + '\r\n';
}
