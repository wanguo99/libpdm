let websocket;

function log(message) {
    const now = new Date().toISOString();
    document.getElementById('log').innerHTML += `[${now}] ${message}\n`;
    document.getElementById('log').scrollTop = document.getElementById('log').scrollHeight;
}

async function getWsUri() {
    try {
        const response = await fetch('/api/get_ws_uri');
        if (!response.ok) {
            throw new Error('Network response was not ok');
        }
        const data = await response.json();
        return data.wsUri;
    } catch (error) {
        log(`Error fetching WebSocket URI: ${error.message}`);
        throw error;
    }
}

function connectWebSocket(wsUri) {
    websocket = new WebSocket(wsUri);

    websocket.onopen = function(event) {
        log("Connected to WebSocket server.");
    };

    websocket.onmessage = function(event) {
        log(`Message from server: ${event.data}`);
    };

    websocket.onerror = function(event) {
        log(`WebSocket error observed: ${event}`);
    };

    websocket.onclose = function(event) {
        log(`WebSocket closed. Code: ${event.code}, Reason: ${event.reason}`);
    };
}

export { log, getWsUri, connectWebSocket };