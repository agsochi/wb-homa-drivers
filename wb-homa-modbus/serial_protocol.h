#pragma once

#include <string>
#include <memory>
#include <exception>
#include <stdint.h>

#include "portsettings.h"
#include "regformat.h"

class TSerialProtocolException: public std::exception {
public:
    TSerialProtocolException(std::string message): Message("Serial protocol error: " + message) {}
    const char* what () const throw ()
    {
        return Message.c_str();
    }

private:
    std::string Message;
};

class TSerialProtocolTransientErrorException: public TSerialProtocolException {
public:
    TSerialProtocolTransientErrorException(std::string message): TSerialProtocolException(message) {}
};

class TAbstractSerialPort: public std::enable_shared_from_this<TAbstractSerialPort> {
public:
    virtual ~TAbstractSerialPort();
    virtual void SetDebug(bool debug) = 0;
    virtual void Open() = 0;
    virtual void Close() = 0;
    virtual bool IsOpen() const = 0;
    virtual void CheckPortOpen() = 0;
    virtual void WriteBytes(const uint8_t* buf, int count) = 0;
    virtual uint8_t ReadByte() = 0;
    virtual int ReadFrame(uint8_t* buf, int count) = 0;
    virtual void SkipNoise() =0;
};

typedef std::shared_ptr<TAbstractSerialPort> PAbstractSerialPort;

class TSerialPort: public TAbstractSerialPort {
public:
    TSerialPort(const TSerialPortSettings& settings);
    ~TSerialPort();
    void SetDebug(bool debug);
    void CheckPortOpen();
    void WriteBytes(const uint8_t* buf, int count);
    uint8_t ReadByte();
    int ReadFrame(uint8_t* buf, int count);
    void SkipNoise();
    void Open();
    void Close();
    bool IsOpen() const;

private:
    void SerialPortSetup();
    bool Select(int ms);

    TSerialPortSettings Settings;
    bool Debug;
    int Fd;
    const int NoiseTimeoutMs = 10;
    const int FrameTimeoutMs = 15;
};

class TSerialProtocol: public std::enable_shared_from_this<TSerialProtocol> {
public:
    TSerialProtocol(PAbstractSerialPort port);
    virtual ~TSerialProtocol();

    void SetDebug(bool debug) { SerialPort->SetDebug(debug); }
    void Open() { SerialPort->Open(); }
    void Close() { SerialPort->Close(); }
    bool IsOpen() const { return SerialPort->IsOpen(); }

    virtual uint64_t ReadRegister(uint32_t mod, uint32_t address, RegisterFormat fmt) = 0;
    virtual void WriteRegister(uint32_t mod, uint32_t address, uint64_t value, RegisterFormat fmt) = 0;
    // XXX FIXME: leaky abstraction (need to refactor)
    // Perhaps add 'brightness' register format
    virtual void SetBrightness(uint32_t mod, uint32_t address, uint8_t value) = 0;
    virtual void EndPollCycle();

protected:
    PAbstractSerialPort Port() { return SerialPort; }

private:
    PAbstractSerialPort SerialPort;
};

typedef std::shared_ptr<TSerialProtocol> PSerialProtocol;