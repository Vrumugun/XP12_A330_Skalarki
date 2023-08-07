#pragma once

int InitCommunication(const char* first_buffer, int first_buffer_len);
int ShutdownCommunication();
void PushMessage(const char* msg, int len);
