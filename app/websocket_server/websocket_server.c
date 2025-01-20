#include <libwebsockets.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h> // for malloc and free
#include <cjson/cJSON.h> // 包含 cJSON 库

static int destroy_flag = 0;

void sigint_handler(int sig) {
    destroy_flag = 1;
}

// Function to send a reply back to the client with the specified format
static void ws_txrx_send_reply(struct lws *wsi, const char *type) {
    if (!type) {
        fprintf(stderr, "Invalid type for sending reply.\n");
        return;
    }

    // Create the response message with the specified format
    char response[256];
    snprintf(response, sizeof(response), "[reply]: received %s message", type);

    // Write the response back to the client
    lws_write(wsi, (unsigned char *)response, strlen(response), LWS_WRITE_TEXT);
}

// Function to handle SIMPLE_TEXT messages
static void ws_txrx_handle_simple_text(struct lws *wsi, cJSON *payload_item) {
    cJSON *message_item = cJSON_GetObjectItemCaseSensitive(payload_item, "message");

    if (cJSON_IsString(message_item)) {
        printf("Extracted SIMPLE_TEXT message: %s\n", message_item->valuestring);
        ws_txrx_send_reply(wsi, "SIMPLE_TEXT");
    } else {
        printf("Invalid SIMPLE_TEXT payload.\n");
    }
}

// Function to handle MQTT_EVENT messages
static void ws_txrx_handle_mqtt_event(struct lws *wsi, cJSON *payload_item) {
    cJSON *topic_item = cJSON_GetObjectItemCaseSensitive(payload_item, "topic");
    cJSON *index_item = cJSON_GetObjectItemCaseSensitive(payload_item, "index");
    cJSON *state_item = cJSON_GetObjectItemCaseSensitive(payload_item, "state");

    if (cJSON_IsString(topic_item) && cJSON_IsNumber(index_item) && cJSON_IsString(state_item)) {
        const char *topic = topic_item->valuestring;
        int index = index_item->valueint;
        const char *state = state_item->valuestring;

        printf("Extracted MQTT_EVENT topic: %s\n", topic);
        printf("Extracted MQTT_EVENT index: %d\n", index);
        printf("Extracted MQTT_EVENT state: %s\n", state);

        ws_txrx_send_reply(wsi, "MQTT_EVENT");
    } else {
        printf("Invalid MQTT_EVENT payload.\n");
    }
}

// Main callback function for handling WebSocket events
static int ws_txrx_callback(struct lws *wsi,
                                   enum lws_callback_reasons reason,
                                   void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_RECEIVE: {
            char buffer[LWS_PRE + 512];
            memcpy(buffer + LWS_PRE, in, len);
            buffer[LWS_PRE + len] = '\0';

            printf("Received message: %s\n", buffer + LWS_PRE);

            // Parse the JSON string directly here
            cJSON *root = cJSON_ParseWithLength(buffer + LWS_PRE, len);
            if (!root) {
                fprintf(stderr, "Error parsing JSON: [%s]\n", cJSON_GetErrorPtr());
                break;
            }

            cJSON *type_item = cJSON_GetObjectItemCaseSensitive(root, "type");
            cJSON *payload_item = cJSON_GetObjectItemCaseSensitive(root, "payload");

            if (!cJSON_IsString(type_item) || !payload_item) {
                cJSON_Delete(root);
                fprintf(stderr, "Invalid or missing 'type' or 'payload' field.\n");
                break;
            }

            const char *type = type_item->valuestring;

            // Handle different types of messages based on the 'type' field
            if (strcmp(type, "SIMPLE_TEXT") == 0) {
                ws_txrx_handle_simple_text(wsi, payload_item);
            } else if (strcmp(type, "MQTT_EVENT") == 0) {
                ws_txrx_handle_mqtt_event(wsi, payload_item);
            } else {
                printf("Unsupported message type: %s\n", type);
            }

            cJSON_Delete(root);
            break;
        }
        default:
            break;
    }
    return 0;
}

static struct lws_protocols protocols[] = {
    { "rxtx-protocol", ws_txrx_callback, 0, 0 },
    { NULL, NULL, 0, 0 }
};

int main() {
    struct lws_context_creation_info info;
    struct lws_context *context;

    memset(&info, 0, sizeof(info));
    info.port = 8080;
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;

    context = lws_create_context(&info);
    if (context == NULL) {
        fprintf(stderr, "lws_create_context failed\n");
        return -1;
    }

    signal(SIGINT, sigint_handler);

    while (!destroy_flag) {
        lws_service(context, 50);
        usleep(10000);
    }

    lws_context_destroy(context);
    printf("Server stopped.\n");

    return 0;
}