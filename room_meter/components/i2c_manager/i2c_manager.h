// -*- c-file-style: "Stroustrup"; eval: (auto-complete-mode) -*-                                          
#include <mutex>


#include "driver/i2c.h"

class i2c_manager
{
private:
    i2c_manager();

private:
    static i2c_manager *m_instance;
    std::mutex m_lock;
    i2c_cmd_handle_t m_cmd;
    
public:
    static i2c_manager* instance();

    i2c_cmd_handle_t GetCmdHandle();
    void ReleaseCmdHandle(const i2c_cmd_handle_t &cmd);
};
