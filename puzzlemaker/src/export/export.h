#pragma once

void clearMessage();
char* getMessage();
void writeMessage(const char* data);
void writeMessagef(const char* fmt, ...);
void writeMessagen(const char* data, int len);
void exportVoxels();
void exportMapSettings();
