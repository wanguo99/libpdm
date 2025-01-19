// js/utils.js

const LOG_LEVELS = {
    ERROR: 0,
    WARN: 1,
    INFO: 2,
    DEBUG: 3,
};

let currentLogLevel = LOG_LEVELS.INFO;

if (typeof process !== 'undefined' && process.env) {
    const envLogLevel = process.env.LOG_LEVEL?.toUpperCase();
    if (envLogLevel in LOG_LEVELS) {
        currentLogLevel = LOG_LEVELS[envLogLevel];
    }
}

function formatLog(level, ...args) {
    const timestamp = new Date().toISOString();
    return `[${timestamp}] [${level}] ${args.map(arg => JSON.stringify(arg)).join(' ')}`;
}

function logIfEnabled(levelName, levelValue, ...args) {
    if (levelValue <= currentLogLevel) {
        console[levelName.toLowerCase()](formatLog(levelName, ...args));
    }
}

export function logError(...args) {
    logIfEnabled('ERROR', LOG_LEVELS.ERROR, ...args);
}

export function logWarn(...args) {
    logIfEnabled('WARN', LOG_LEVELS.WARN, ...args);
}

export function logInfo(...args) {
    logIfEnabled('INFO', LOG_LEVELS.INFO, ...args);
}

export function logDebug(...args) {
    logIfEnabled('DEBUG', LOG_LEVELS.DEBUG, ...args);
}