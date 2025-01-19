// js/utils.js

class Logger {
    constructor(logOutputId) {
        this.logOutputId = logOutputId;
        this.logOutputElement = logOutputId ? document.getElementById(logOutputId) : null;
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

    updateLogOutput(message, level) {
        if (!this.logOutputElement) return;

        const logEntry = document.createElement('div');
        logEntry.className = `log-entry log-level-${level.toLowerCase()}`;

        logEntry.textContent = message;

        if (this.logOutputElement.firstChild) {
            this.logOutputElement.insertBefore(logEntry, this.logOutputElement.firstChild);
        } else {
            this.logOutputElement.appendChild(logEntry);
        }
    }

    formatLog(level, ...args) {
        // 创建一个新的 Date 实例并获取当前时间
        const now = new Date();

        // 使用 Intl.DateTimeFormat 进行格式化，确保兼容性和本地化
        const options = {
            year: 'numeric',
            month: '2-digit',
            day: '2-digit',
            hour: '2-digit',
            minute: '2-digit',
            second: '2-digit',
            weekday: 'short', // 使用星期的简短英文表示
            timeZoneName: 'short' // 时区缩写
        };

        const formatter = new Intl.DateTimeFormat('en-US', options); // 强制使用美式英语格式
        const parts = formatter.formatToParts(now);
        const timestampParts = {
            year: '',
            month: '',
            day: '',
            hour: '',
            minute: '',
            second: '',
            weekday: '',
            timeZoneName: ''
        };

        for (const part of parts) {
            if (part.type in timestampParts) {
                timestampParts[part.type] = part.value;
            }
        }

        // 组装时间戳字符串
        const timestamp = `${timestampParts.year}-${timestampParts.month}-${timestampParts.day} ${timestampParts.hour}:${timestampParts.minute}:${timestampParts.second} (${timestampParts.weekday}, ${timestampParts.timeZoneName})`;

        // 将日志级别和消息内容转换为字符串
        return `[${timestamp}] - [${level}] ： ${args.map(arg => JSON.stringify(arg)).join(' ')}`;
    }
}

export { Logger };