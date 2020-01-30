#ifndef GLOBALSTATEHANDLERS_HPP
#define GLOBALSTATEHANDLERS_HPP

extern bool FPScounter;
//extern bool

bool GetFPScounter();
void SetFPScounter(bool state);

struct boolLink {
    std::string ID;
    bool *ptr;
};

extern struct boolLink boolLinkArray[];
extern const uint boolLinkArraySize;

#endif // GLOBALSTATEHANDLERS_HPP

