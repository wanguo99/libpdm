// js/ws_manager.js

import { logInfo, logError } from './utils.js';

class WebSocketManager {
    constructor() {
        this.websocket = null;
        this.messageCallbacks = [];
    }

    async connect(wsUri) {
        return new Promise((resolve, reject) => {
            if (this.websocket && this.websocket.readyState === WebSocket.OPEN) {
                logInfo("WebSocket is already open.");
                resolve(this.websocket);
                return;
            }

            this.websocket = new WebSocket(wsUri);

            this.websocket.onopen = () => {
                logInfo("Connected to WebSocket server.");
                resolve(this.websocket);
            };

            this.websocket.onerror = (error) => {
                logError(`WebSocket error observed: ${error.message}`);
                reject(error);
            };

            this.websocket.onclose = (event) => {
                logInfo(`WebSocket closed. Code: ${event.code}, Reason: ${event.reason}`);
                reject(new Error('WebSocket connection closed'));
            };

            this.websocket.onmessage = (event) => {
                const message = event.data;
                logInfo(`Received message: ${message}`);
                this.messageCallbacks.forEach(callback => callback(message));
            };
        });
    }

    sendMessage(message) {
        if (this.websocket && this.websocket.readyState === WebSocket.OPEN) {
            this.websocket.send(message);
            logInfo(`Sent message: ${message}`);
        } else {
            logError("WebSocket is not open. Message not sent.");
        }
    }

    onMessage(callback) {
        this.messageCallbacks.push(callback);
    }

    disconnect() {
        if (this.websocket && this.websocket.readyState === WebSocket.OPEN) {
            this.websocket.close();
            logInfo("WebSocket disconnected.");
        }
    }
}

export { WebSocketManager };