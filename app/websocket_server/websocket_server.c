#include <libwebsockets.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

static int destroy_flag = 0;

void sigint_handler(int sig) {
    destroy_flag = 1;
}

static int callback_websocket(struct lws *wsi,
                              enum lws_callback_reasons reason,
                              void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_RECEIVE: {
            char buffer[LWS_PRE + 512];
            memcpy(buffer + LWS_PRE, in, len);
            buffer[LWS_PRE + len] = '\0';

            printf("Received message: %s\n", buffer + LWS_PRE);

            // Add a prefix to the received message before echoing it back
            const char *prefix = "[reply] : ";
            size_t prefix_len = strlen(prefix);
            if (len + prefix_len > sizeof(buffer) - LWS_PRE - 1) {
                // If the message with prefix is too long, truncate it
                len = sizeof(buffer) - LWS_PRE - prefix_len - 1;
            }

            memmove(buffer + LWS_PRE + prefix_len, buffer + LWS_PRE, len);
            memcpy(buffer + LWS_PRE, prefix, prefix_len);
            len += prefix_len;

            // Echo the modified message back to the client
            lws_write(wsi, (unsigned char *)buffer + LWS_PRE, len, LWS_WRITE_TEXT);
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