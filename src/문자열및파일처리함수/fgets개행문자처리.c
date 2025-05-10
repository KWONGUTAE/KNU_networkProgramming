fgets(input_message, (BUF_SIZE), stdin);

size_t len = strlen(input_message);
if (len > 0 && input_message[len - 1] == '\n') {
    input_message[len - 1] = '\0';
}