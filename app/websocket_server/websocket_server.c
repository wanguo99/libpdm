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

#define PDM_SWITCH_LED_ON "echo -s 1 > /dev/pdm_client/pdm_switch.0"
#define PDM_SWITCH_LED_OFF "echo -s 0 > /dev/pdm_client/pdm_switch.0"


static int callback_websocket(struct lws *wsi,
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
                fprintf(stderr, "Error before: [%s]\n", cJSON_GetErrorPtr());
                break;
            }

            cJSON *type = cJSON_GetObjectItemCaseSensitive(root, "type");
            cJSON *topic = cJSON_GetObjectItemCaseSensitive(root, "topic");
            cJSON *index = cJSON_GetObjectItemCaseSensitive(root, "index");
            cJSON *payload = cJSON_GetObjectItemCaseSensitive(root, "payload");

            char *extracted_topic = NULL;
            char *extracted_index = NULL;
            char *extracted_payload = NULL;

            // Check if the type is "publish_mqtt_event" and both topic and payload are strings
            if (cJSON_IsString(topic) && cJSON_IsString(payload) &&
                (type != NULL) && (strcmp(type->valuestring, "publish_mqtt_event") == 0)) {

                // Extract the values directly into local variables
                extracted_topic = strdup(topic->valuestring); // 使用 strdup 复制字符串并分配内存
                extracted_payload = strdup(payload->valuestring);
                extracted_index = strdup(index->valuestring);

                if (extracted_topic == NULL || extracted_index == NULL || extracted_payload == NULL) {
                    // 如果其中一个复制失败，确保释放已分配的内存
                    free(extracted_topic);
                    free(extracted_index);
                    free(extracted_payload);
                    cJSON_Delete(root);
                    fprintf(stderr, "Memory allocation failed.\n");
                    break;
                }

                printf("Extracted topic: %s\n", extracted_topic);
                printf("Extracted index: %s\n", extracted_index);
                printf("Extracted payload: %s\n", extracted_payload);

				if (!strcmp(extracted_topic, "SWITCH") && !strcmp(extracted_payload, "ON")) {
					system(PDM_SWITCH_LED_ON);
				}
				else if (!strcmp(extracted_topic, "SWITCH") && !strcmp(extracted_payload, "OFF")) {
					system(PDM_SWITCH_LED_OFF);
				}

                // Add a prefix to the received message before echoing it back
                const char *prefix = "[reply] : ";
                size_t prefix_len = strlen(prefix);
                size_t new_msg_len = strlen(prefix) + strlen(extracted_topic) + strlen(extracted_payload) + 2; // +2 for ": " separator

                if (new_msg_len > sizeof(buffer) - LWS_PRE - 1) {
                    // If the message with prefix is too long, truncate it
                    new_msg_len = sizeof(buffer) - LWS_PRE - 1;
                }

                snprintf(buffer + LWS_PRE, sizeof(buffer) - LWS_PRE, "%s%s: %s", prefix, extracted_topic, extracted_payload);
                len = strlen(buffer + LWS_PRE);

                // Echo the modified message back to the client
                lws_write(wsi, (unsigned char *)buffer + LWS_PRE, len, LWS_WRITE_TEXT);
            } else {
                printf("Message does not contain expected fields or type mismatch.\n");
            }

            // Clean up cJSON object and free allocated memory
            cJSON_Delete(root);
            free(extracted_topic);
            free(extracted_payload);
            break;
        }
        default:
            break;
    }
    return 0;
}

static struct lws_protocols protocols[] = {
    { "echo-protocol", callback_websocket, 0, 0 },
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
        fprintf(stderr, "libwebsockets init failed\n");
        return -1;
    }

    signal(SIGINT, sigint_handler);

    while (!destroy_flag) {
        lws_service(context, 50);
        usleep(10000); // Sleep for 10ms
    }

    lws_context_destroy(context);
    printf("Server stopped.\n");

    return 0;
}