#ifndef GLOBALSTATEHANDLERS_HPP
#define GLOBALSTATEHANDLERS_HPP

extern bool FPScounter;
//extern bool

bool GetFPScounter();
void SetFPScounter(bool state);

#define LOOP_MODE_MENU 1
#define LOOP_MODE_NETWORK 2
#define LOOP_MODE_EDIT 3
#define LOOP_MODE_PLAY 4
#define LOOP_MODE_LEGACY 5

int getLoopMode();
void setLoopMode(int val);

//bool
struct boolLink {
    std::string ID;
    bool *ptr;
};
extern struct boolLink boolLinkArray[];
extern const uint boolLinkArraySize;

//select
struct selectLink {
    std::string ID;
    int *ptr;
};
extern struct selectLink selectLinkArray[];
extern const uint selectLinkArraySize;

#endif // GLOBALSTATEHANDLERS_HPP

