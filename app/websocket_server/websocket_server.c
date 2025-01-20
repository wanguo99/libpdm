#include <libwebsockets.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h> // for malloc and free
#include <fcntl.h>
#include <sys/stat.h>
#include <cjson/cJSON.h> // 包含 cJSON 库

#define WS_MQTT_CONFIG_FILE "/etc/websocket/mqtt_conf.json"

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

// Function to handle MQTT_PUBLISH_EVENT messages
static void ws_txrx_handle_mqtt_publish_event(struct lws *wsi, cJSON *payload_item) {
    cJSON *topic_item = cJSON_GetObjectItemCaseSensitive(payload_item, "topic");
    cJSON *index_item = cJSON_GetObjectItemCaseSensitive(payload_item, "index");
    cJSON *state_item = cJSON_GetObjectItemCaseSensitive(payload_item, "state");

    if (cJSON_IsString(topic_item) && cJSON_IsNumber(index_item) && cJSON_IsString(state_item)) {
        const char *topic = topic_item->valuestring;
        int index = index_item->valueint;
        const char *state = state_item->valuestring;

        printf("Extracted MQTT_PUBLISH_EVENT topic: %s\n", topic);
        printf("Extracted MQTT_PUBLISH_EVENT index: %d\n", index);
        printf("Extracted MQTT_PUBLISH_EVENT state: %s\n", state);

        ws_txrx_send_reply(wsi, "MQTT_PUBLISH_EVENT");
    } else {
        printf("Invalid MQTT_PUBLISH_EVENT payload.\n");
    }
}

static void load_and_send_mqtt_messages_from_file(struct lws *wsi, const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return;
    }

    // 获取文件大小并分配相应内存
    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *content = malloc(fsize + 1);
    if (!content) {
        fclose(file);
        fprintf(stderr, "Memory allocation failed.\n");
        return;
    }

    size_t read_size = fread(content, 1, fsize, file);
    content[read_size] = '\0';
    fclose(file);

    cJSON *root = cJSON_Parse(content);
    free(content); // Free the allocated memory after parsing

    if (!root) {
        fprintf(stderr, "Error parsing JSON: [%s]\n", cJSON_GetErrorPtr());
        return;
    }

    cJSON *item = NULL;
    cJSON_ArrayForEach(item, root) {
        cJSON *topic_item = cJSON_GetObjectItemCaseSensitive(item, "topic");
        cJSON *payload_item = cJSON_GetObjectItemCaseSensitive(item, "payload");

        if (cJSON_IsString(topic_item) && cJSON_IsObject(payload_item)) {
            // 构造要发送的 JSON 对象
            cJSON *send_obj = cJSON_CreateObject();
            cJSON_AddStringToObject(send_obj, "topic", topic_item->valuestring);

            // 只复制 payload 部分，而不删除原始数据
            cJSON *payload_copy = cJSON_Duplicate(payload_item, 1);
            if (payload_copy) {
                cJSON_AddItemToObject(send_obj, "payload", payload_copy);

                // 打印 JSON 字符串并发送
                char *send_str = cJSON_PrintUnformatted(send_obj);
                if (send_str) {
                    printf("Sending MQTT message:\n%s\n", send_str);
                    lws_write(wsi, (unsigned char *)send_str, strlen(send_str), LWS_WRITE_TEXT);
                    free(send_str); // 正确释放打印出来的 JSON 字符串
                } else {
                    fprintf(stderr, "Failed to print JSON string.\n");
                }

                cJSON_Delete(send_obj); // 删除构造的对象
            } else {
                fprintf(stderr, "Failed to duplicate payload.\n");
                cJSON_Delete(send_obj); // 记得删除 send_obj
            }
        } else {
            fprintf(stderr, "Invalid or missing 'topic' or 'payload' field in item.\n");
        }
    }

    cJSON_Delete(root); // 最后删除解析得到的根对象
}



static int ws_txrx_callback(struct lws *wsi,
                            enum lws_callback_reasons reason,
                            void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_RECEIVE: {
            char buffer[LWS_PRE + len + 1];
            memcpy(buffer + LWS_PRE, in, len);
            buffer[LWS_PRE + len] = '\0';

            printf("Received message: %s\n", buffer + LWS_PRE);

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
            } else if (strcmp(type, "MQTT_PUBLISH_EVENT") == 0) {
                ws_txrx_handle_mqtt_publish_event(wsi, payload_item);
            } else if (strcmp(type, "MQTT_GET_CONFIG") == 0) {
                load_and_send_mqtt_messages_from_file(wsi, WS_MQTT_CONFIG_FILE);
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
    printf("Server stopped gracefully.\n");

    return 0;
}
