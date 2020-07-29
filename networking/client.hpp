struct packet
{
    char name[128];
    int ptl;
    unsigned long long time;
    char extra[BUFSIZE];
};

void composeMsg(char msg[], int protocol, char extra[] = "none");
int connectToServer(struct sockaddr_in addr, int *id, struct packet *msg);
int makePacket(char msg[], struct packet *out);
int makeSocket();
unsigned long long getMilliSeconds();
void setPositions(struct entities all[], char extra[]);
