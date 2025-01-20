import { Logger } from './utils.js';

class WebSocketManager {
    constructor(wsUri) {
        if (!wsUri) {
            throw new Error("WebSocket URI is required.");
        }
        this.websocket = null;
        this.messageCallbacks = [];
        this.wsUri = wsUri;

        this.logger = new Logger('log-output');
    }

    async connect() {
        return new Promise((resolve, reject) => {
            if (this.websocket && this.websocket.readyState === WebSocket.OPEN) {
                this.logger.info("WebSocket is already open.");
                resolve(this.websocket);
                return;
            }

            this.websocket = new WebSocket(this.wsUri, 'rxtx-protocol');

            this.websocket.onopen = () => {
                this.logger.info("Connected to WebSocket server.");
                resolve(this.websocket);
            };

            this.websocket.onerror = (error) => {
                this.logger.error(`WebSocket error observed: ${error.message}`);
                reject(error);
            };

            this.websocket.onclose = (event) => {
                this.logger.info(`WebSocket closed. Code: ${event.code}, Reason: ${event.reason}`);
                reject(new Error('WebSocket connection closed'));
            };

            this.websocket.onmessage = (event) => {
                const message = event.data;
                this.logger.info(`Received message: ${message}`);
                this.messageCallbacks.forEach(callback => callback(message));
            };
        });
    }

    disconnect() {
        if (this.websocket && this.websocket.readyState === WebSocket.OPEN) {
            this.websocket.close();
            this.logger.info("WebSocket disconnected.");
        }
    }

    sendMessage(type, payload) {
        return new Promise((resolve, reject) => {
            // 构造消息对象
            const messageObject = {
                type: type,
                payload: payload
            };

            // 将消息对象转换为 JSON 字符串
            const messageToSend = JSON.stringify(messageObject);

            if (this.websocket && this.websocket.readyState === WebSocket.OPEN) {
                try {
                    this.websocket.send(messageToSend);
                    this.logger.info(`Sent message: ${messageToSend}`);
                    resolve(); // 成功发送消息后解析 Promise
                } catch (error) {
                    this.logger.error("An error occurred while sending the message.");
                    reject(error); // 发送失败时拒绝 Promise
                }
            } else {
                this.logger.error("WebSocket is not open. Message not sent.");
                reject(new Error("WebSocket is not open. Message not sent.")); // WebSocket 未打开时拒绝 Promise
            }
        });
    }

    onMessage(callback) {
        this.messageCallbacks.push(callback);
    }
}

export { WebSocketManager };