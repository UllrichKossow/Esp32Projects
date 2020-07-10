// -*- c-file-style: "Stroustrup"; eval: (auto-complete-mode) -*-                                         
#ifndef __PLOT_H__
#define __PLOT_H__



class Plot
{
public:
    Plot(int div);
    ~Plot();

    bool PushValue(double v);

private:
    static const int m_len;
    int m_idx;
    int m_div;
    int m_val_cnt;
    double *m_values;
    double m_current;
};


#endif
