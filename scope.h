#ifndef SCOPE_H
#define SCOPE_H

#define NO_NAME -1

extern bool analysisError;

void defineName(int name);
bool findName(int name);
void startBlock();
void finishBlock();

#endif
