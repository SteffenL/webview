const fs = require("node:fs");

function parseRow(line) {
    const row = [];
    let quoted = false;
    let chars = [];
    for (let i = 0; i < line.length; ++i) {
        const c = line.charAt(i);
        const next = line.charAt(i + 1);
        if (quoted) {
            if (c !== '"') {
                chars.push(line.charCodeAt(i));
                continue;
            }
            if (next === '"') {
                chars.push(line.charCodeAt(i));
                ++i;
                continue;
            }
            quoted = false;
            continue;
        }
        switch (c) {
            case '"':
                if (next === '"') {
                    chars.push(line.charCodeAt(i));
                    ++i;
                    continue;
                }
                quoted = true;
                break;
            case ",":
                row.push(String.fromCodePoint(...chars));
                chars = [];
                break;
            default:
                chars.push(line.charCodeAt(i));
                break;
        }
    }
    if (chars.length > 0) {
        row.push(String.fromCodePoint(...chars));
    }
    return row;
}

function parseRows(lines) {
    return lines.map(parseRow);
}

function extractLines(content) {
    return content.split("\n").filter(s => s.length > 0);
}

function processRows(rows) {
    if (rows.length === 0) {
        return [];
    }
    const result = [];
    const header = rows[0];
    if (rows.length === 1) {
        return result;
    }
    for (let i = 1; i < rows.length; ++i) {
        const rowObject = {};
        for (const j in header) {
            rowObject[header[j]] = rows[i][j];
        }
        result.push(rowObject);
    }
    return result;
}

export function loadString(content) {
    return processRows(parseRows(extractLines(content)));
}

export function loadFile(filePath) {
    return loadString(fs.readFileSync(filePath, "utf-8"));
}
