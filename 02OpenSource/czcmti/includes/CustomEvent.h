#ifndef __CUSTOMEVENT_H__
#define __CUSTOMEVENT_H__
#include <QEvent>
#include <QVariant>
#include <QColor>
#include "czcmtidefs.h"

enum E_CustomEventId {
    CustomEventId_ExceptionEvent = QEvent::User + 1,
    CustomEventId_LoggerEvent,
    CustomEventId_FpsEvent,
    CustomEventId_RenderFrameEvent,
    CustomEventId_ChannelStateEvent,
    CustomEventId_ActionEvent,
    CustomEventId_HintUpdateEvent,
    CustomEventId_TestResultEvent,
    CustomEventId_AsyncInvokeEvent,
    CustomEventId_MachineRequestEvent,
    CustomEventId_MachineResponseEvent,
    CustomEventId_MachineAntiShakeEvent,
    CustomEventId_MountShareDiskEvent,
    CustomEventId_TestItemStateChangedEvent,
    CustomEventId_DiskSizeChangedEvent,
};

class ExceptionEvent : public QEvent
{
public:
    enum ExceptionType {
        Exception_SystemSignal,
        Exception_ApplicationError,
    };
    explicit ExceptionEvent(ExceptionType exType, int code, int categoryCode = 0, const QString &message = "") :
        QEvent(type()),
        ExType(exType),
        Code(code),
        CategoryCode(categoryCode),
        Message(message)
    {}
    static QEvent::Type type() { return static_cast<Type>(CustomEventId_ExceptionEvent); }

    const ExceptionType ExType;
    const int Code;
    int CategoryCode;
    const QString Message;
};

class LoggerEvent : public QEvent
{
public:
    enum E_LogType {
        Log_Fail,
        Log_Pass,
        Log_Info,
        Log_Debug,
    };
    explicit LoggerEvent(const QString &text, E_LogType logType) :
        QEvent(type()),
        Text(text),
        LogType(logType)
    {
        switch (logType)
        {
        case Log_Fail:
            Rgb = QColor(Qt::red).rgb();
            break;
        case Log_Info:
            Rgb = QColor(Qt::darkYellow).rgb();
            break;
        case Log_Pass:
            Rgb = QColor(Qt::darkGreen).rgb();
            break;
        case Log_Debug:
        default:
            Rgb = QColor(Qt::black).rgb();
            break;
        }
    }
    explicit LoggerEvent(const QString &text, QRgb rgb) :
        QEvent(type()),
        Text(text),
        LogType(Log_Info),
        Rgb(rgb)
    {}
    static QEvent::Type type() { return static_cast<Type>(CustomEventId_LoggerEvent); }

    const QString Text;
    const E_LogType LogType;
    QRgb Rgb;
};

class FpsEvent : public QEvent
{
public:
    enum E_FpsType {
        Fps_Capture,
        Fps_Transfer,
        Fps_Test,
        Fps_Display
    };
    explicit FpsEvent(const float fps, E_FpsType fpsType) :
        QEvent(type()),
        Fps(fps),
        FpsType(fpsType)
    {}
    static QEvent::Type type() { return static_cast<Type>(CustomEventId_FpsEvent); }

    const float Fps;
    const E_FpsType FpsType;
};

class RenderFrameEvent : public QEvent
{
public:
    explicit RenderFrameEvent(const uchar *rgbBuffer, const T_FrameParam &frameParam) :
        QEvent(type()),
        RgbBuffer(rgbBuffer),
        FrameParam(frameParam)
    {}
    static QEvent::Type type() { return static_cast<Type>(CustomEventId_RenderFrameEvent); }

    const uchar* RgbBuffer;
    const T_FrameParam FrameParam;
};

class ChannelStateEvent : public QEvent
{
public:
    enum E_ChannelState {
        State_PowerOff = 0,
        State_PowerOnStreamOff,
        State_PowerOnStreamOn,  // stream & capture thread on
        State_Playing,          // transmit & decode
    };
    explicit ChannelStateEvent(E_ChannelState channelState) :
        QEvent(type()),
        ChannelState(channelState)
    {}
    static QEvent::Type type() { return static_cast<Type>(CustomEventId_ChannelStateEvent); }

    const E_ChannelState ChannelState;
};

enum E_ActionId {
    Action_Start,
    Action_Stop,
    Action_DispTextPanel,
    Action_DispTestItemPanel,
    Action_DispTestResultPanel,
};
struct ActionEvent : public QEvent
{
public:
    explicit ActionEvent(int actionId, bool checked = false) :
        QEvent(type()),
        ActionId(actionId),
        Checked(checked)
    {}
    static QEvent::Type type() { return static_cast<Type>(CustomEventId_ActionEvent); }

    int ActionId;
    bool Checked;
};

class TestResultEvent : public QEvent
{
public:
    explicit TestResultEvent(int ec, const QString &caption, const QList<QStringList> &resultTable) :
        QEvent(type()),
        ErrorCode(ec),
        Caption(caption),
        ResultTable(resultTable)
    {}
    static QEvent::Type type() { return static_cast<Type>(CustomEventId_TestResultEvent); }

    int ErrorCode;
    const QString Caption;
    const QList<QStringList> ResultTable;
};

struct T_AsyncContext {
    uint SessionId;
    QObject *EventReceiver;
};

class AsyncInvokeEvent : public QEvent
{
public:
    enum E_AsyncCommand {
        AsyncCommand_OpenCamera = 0,
        AsyncCommand_CloseCamera,

        AsyncCommand_Exception = (0x01 << 16),
    };
public:
    explicit AsyncInvokeEvent(uint sessionId, int ec, const QString &text = "") :
        QEvent(type()),
        SessionId(sessionId),
        ErrorCode(ec),
        Text(text)
    {}
    static QEvent::Type type() { return static_cast<Type>(CustomEventId_AsyncInvokeEvent); }

    const uint SessionId; // Application: high 16bit, Action: low 16bit
    const int ErrorCode;
    const QString Text;
};

class MachineRequestEvent : public QEvent
{
public:
    MachineRequestEvent() :
        QEvent(type())
    {}
    explicit MachineRequestEvent(uint command, uint subStation, uint chnIdx, const QString &parameter = "") :
        QEvent(type()),
        Command(command),
        SubStation(subStation),
        ChnIdx(chnIdx),
        Parameter(parameter)
    {}
    MachineRequestEvent &operator=(const MachineRequestEvent &other)
    {
        this->Command = other.Command;
        this->SubStation = other.SubStation;
        this->ChnIdx = other.ChnIdx;
        this->Parameter = other.Parameter;
        return *this;
    }
    static QEvent::Type type() { return static_cast<Type>(CustomEventId_MachineRequestEvent); }

    uint Command;
    uint SubStation;
    uint ChnIdx;
    QString Parameter;
};

class MachineAntiShakeEvent : public QEvent
{
public:
    explicit MachineAntiShakeEvent(uint id, uint state) :
        QEvent(type()),
        m_id(id),
        m_state(state)
    {}
    static QEvent::Type type() { return static_cast<Type>(CustomEventId_MachineAntiShakeEvent); }

    const uint m_id;
    const uint m_state;
};

class MachineResponseEvent : public QEvent
{
public:
    explicit MachineResponseEvent(uint command, uint subStation, uint chnIdx, uint categoryCode) :
        QEvent(type()),
        Command(command),
        SubStation(subStation),
        ChnIdx(chnIdx),
        CategoryCode(categoryCode)
    {}
    static QEvent::Type type() { return static_cast<Type>(CustomEventId_MachineResponseEvent); }

    const uint Command;
    const uint SubStation;
    const uint ChnIdx;
    const uint CategoryCode;
};

class TestItemStateChangedEvent : public QEvent
{
public:
    enum E_State {
        State_Initialized = 0,
        State_Finished, // current item
        State_GroupFinished // current group
    };

    explicit TestItemStateChangedEvent(int chnIdx, int groupIdx, int idxInGroup,
                                       E_State state, int errorCode, int elapsed) :
        QEvent(type()),
        ChnIdx(chnIdx),
        GroupIdx(groupIdx),
        IdxInGroup(idxInGroup),
        State(state),
        ErrorCode(errorCode),
        Elapsed(elapsed)
    {}
    static QEvent::Type type() { return static_cast<Type>(CustomEventId_TestItemStateChangedEvent); }

    const int ChnIdx;
    const int GroupIdx;
    const int IdxInGroup;
    const E_State State;
    const int ErrorCode;
    const int Elapsed;
};

class DiskSizeChangedEvent : public QEvent
{
public:
    explicit DiskSizeChangedEvent(const uint64 total, const uint64 available) :
        QEvent(type()),
        Total(total),
        Available(available)
    {}
    static QEvent::Type type() { return static_cast<Type>(CustomEventId_DiskSizeChangedEvent); }

    const uint64 Total;
    const uint64 Available;
};

#endif // __CUSTOMEVENT_H__
