// js/utils.js

class Logger {
    constructor(logOutputId) {
        this.logOutputId = logOutputId;
        this.logOutputElement = logOutputId ? document.getElementById(logOutputId) : null;
    }

    formatLog(level, ...args) {
        const timestamp = new Date().toISOString();
        return `[${timestamp}] [${level}] ${args.map(arg => JSON.stringify(arg)).join(' ')}`;
    }

    updateLogOutput(message, level) {
        if (!this.logOutputElement) return;

        const logEntry = document.createElement('div');
        logEntry.className = `log-entry log-level-${level.toLowerCase()}`;

        logEntry.textContent = message;
        this.logOutputElement.appendChild(logEntry);
        this.logOutputElement.scrollTop = this.logOutputElement.scrollHeight; // 滚动到底部
    }

    info(...args) {
        const message = this.formatLog('INFO', ...args);
        console.info(message);
        this.updateLogOutput(message, 'INFO');
    }

    warn(...args) {
        const message = this.formatLog('WARN', ...args);
        console.warn(message);
        this.updateLogOutput(message, 'WARN');
    }

    error(...args) {
        const message = this.formatLog('ERROR', ...args);
        console.error(message);
        this.updateLogOutput(message, 'ERROR');
    }
}

export { Logger };