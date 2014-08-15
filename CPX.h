void CPXloop();
void CPXsetup();
void CPXaddFunction(char* name, int (*functionPtr)(char*,char*));
void sendPing();
void CPXerrorPin(int);
void CPXokPin(int);
int CPXconnected();
void CPXprint(char* what);
