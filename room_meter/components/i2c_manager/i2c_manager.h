// -*- c-file-style: "Stroustrup"; eval: (auto-complete-mode) -*-                                          
#include <mutex>


class i2c_manager
{
private:
    i2c_manager();

private:
    static i2c_manager *m_instance;
    std::mutex m_lock;
    
public:
    static i2c_manager* instance();

    void begin_use();
    void end_use();
};
