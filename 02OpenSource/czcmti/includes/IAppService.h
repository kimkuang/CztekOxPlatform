#ifndef __IAPPSERVICE_H__
#define __IAPPSERVICE_H__
#include <QByteArray>
#include "ICzPlugin.h"

class IAppService
{
public:
    enum E_ServiceState {
        ServiceState_Stopped = 0,
        ServiceState_Running,
    };
    virtual ~IAppService() {}

    virtual int ShowConfigDialog() = 0;
    virtual int LoadOption() = 0;
    virtual int SaveOption() = 0;
    virtual int RestoreDefaults() = 0;
    virtual int BindChannelContext(T_ChannelContext *context) = 0;
    virtual int Start() = 0;
    virtual int Stop() = 0;
    virtual E_ServiceState GetSeriveState() = 0;
};

#define IAppService_iid "czcmti.IAppService"
Q_DECLARE_INTERFACE(IAppService, IAppService_iid)

#endif /* __IAPPSERVICE_H__ */
