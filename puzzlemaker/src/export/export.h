#pragma once

void clearMessage();
char* getMessage();
void writeMessage(char* data);
void writeMessagef(const char* fmt, ...);
void writeMessagen(char* data, int len);
void exportVoxels();
void exportMapSettings();
