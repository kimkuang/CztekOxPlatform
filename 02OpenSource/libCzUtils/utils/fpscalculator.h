#ifndef FPSCALCULATOR_H
#define FPSCALCULATOR_H
#include <QElapsedTimer>
#include "library_global.h"

class LIBRARY_API FpsCalculator
{
public:
    enum E_CalcCycle {
        CC_ByFrameCounter,
        CC_ByElapsedTime,
    };

    FpsCalculator(E_CalcCycle cc, int val);
    void Reset();
    bool Update(float &fps, bool isRt = false);

private:
    const E_CalcCycle m_calcCycle;
    const int m_cycleVal;
    QElapsedTimer m_elapsedTimer;
    int m_frameCounter;
};

#endif // FPSCALCULATOR_H
