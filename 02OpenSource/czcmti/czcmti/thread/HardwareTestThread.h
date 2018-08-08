#ifndef HARDWARETESTTHREAD_H
#define HARDWARETESTTHREAD_H

#include <QThread>

class HardwareTestThread : public QThread
{
    Q_OBJECT
public:
    HardwareTestThread();
};

#endif // HARDWARETESTTHREAD_H
