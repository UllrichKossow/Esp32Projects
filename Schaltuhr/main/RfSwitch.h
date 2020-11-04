#ifndef RFSWITCH_H
#define RFSWITCH_H

#include <vector>
#include <ctime>

class RfSwitch
{
public:
    RfSwitch();

    void StartSniffing();
    void DumpEdges();
    void Clear();

private:
    std::vector<timespec> m_buffer;
};

#endif // RFSWITCH_H
