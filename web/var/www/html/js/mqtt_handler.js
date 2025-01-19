import { log, connectWebSocket } from './ws-handler.js';

async function publishMqttEvent(topic, payload) {
    log("Checking WebSocket status...");

    if (!websocket || websocket.readyState !== WebSocket.OPEN) {
        log("WebSocket is not open. Attempting to reconnect...");
        try {
            const wsUri = await getWsUri();
            connectWebSocket(wsUri); // 确保等待连接完成
        } catch (error) {
            log("Failed to retrieve WebSocket URI and connect.");
            return;
        }
    }

    log('WebSocket is open, proceeding to send message.');

    if (topic && payload) {
        try {
            const message = JSON.stringify({
                type: 'publish_mqtt_event',
                topic: topic,
                payload: payload
            });
            log(`Constructed message: ${message}`);
            websocket.send(message);
            log(`Sent message: ${message}`);
        } catch (error) {
            log(`Failed to send message: ${error.message}`);
        }
    } else {
        log("Please select a topic and enter a payload.");
    }
}

export { publishMqttEvent };