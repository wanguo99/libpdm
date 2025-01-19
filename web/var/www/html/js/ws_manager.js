// js/ws_manager.js

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

            this.websocket = new WebSocket(this.wsUri);

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

    sendMessage(message) {
        return new Promise((resolve, reject) => {
            if (this.websocket && this.websocket.readyState === WebSocket.OPEN) {
                try {
                    this.websocket.send(message);
                    this.logger.info(`Sent message: ${message}`);
                    resolve(); // 成功发送消息后解析 Promise
                } catch (error) {
                    this.logger.error("WebSocket is not open or an error occurred while sending the message.");
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

    async publishMqttEvent(topic, index, payload) {
        this.logger.info("Checking WebSocket status...");

        // Check if WebSocket is open and try to reconnect if necessary
        if (!this.websocket || this.websocket.readyState !== WebSocket.OPEN) {
            this.logger.info("WebSocket is not open. Attempting to reconnect...");
            try {
                await this.connect();
            } catch (error) {
                this.logger.error(`Failed to reconnect: ${error.message}`);
                return;
            }
        }

        this.logger.info('WebSocket is open, proceeding to send message.');

        if (topic && index !== undefined && payload !== undefined) {
            try {
                const message = JSON.stringify({
                    type: 'publish_mqtt_event',
                    topic: topic,
                    index: index, // Add the index field to the message object
                    payload: payload
                });
                this.logger.info(`Constructed MQTT event message: ${message}`);
                this.sendMessage(message);
                this.logger.info('Message sent successfully.');
            } catch (error) {
                this.logger.error(`Failed to construct or send message: ${error.message}`);
            }
        } else {
            this.logger.error("Please select a topic, an index, and enter a payload.");
        }
    }
}

export { WebSocketManager };
