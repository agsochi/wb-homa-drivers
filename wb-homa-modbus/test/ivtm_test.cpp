#include "testlog.h"
#include "fake_serial_port.h"
#include "serial_connector.h"


class TIVTMProtocolTest: public TLoggedFixture
{
protected:
    void SetUp();
    void TearDown();
    // void EnqueueMilurSessionSetupResponse();
    // void EnqueueMercury230SessionSetupResponse();
    // void VerifyMilurQuery();
    // void VerifyMercuryParamQuery();

    PFakeSerialPort SerialPort;
    PModbusContext Context;
};

void TIVTMProtocolTest::SetUp()
{
    SerialPort = PFakeSerialPort(new TFakeSerialPort(*this));
    Context = TSerialConnector().CreateContext(SerialPort);
    Context->AddDevice(0x0001, "ivtm");
}

void TIVTMProtocolTest::TearDown()
{
    SerialPort.reset();
    Context.reset();
    TLoggedFixture::TearDown();
}

TEST_F(TIVTMProtocolTest, IVTM7MQuery)
{
    Context->SetSlave(0x01);

    // >> 24 30 30 30 31 52 52 30 30 30 30 30 34 61 64 0d
    // << 21 30 30 30 31 52 52 43 45 44 33 44 31 34 31 35 46 0D 
    // temperature == 26.228420

    SerialPort->EnqueueResponse(
        {
            // Session setup response
            '!',                  // header
            '0', '0', '0', '1',   // slave addr
            'R', 'R',             // read response
            'C', 'E', 'D', '3', 'D', '1', '4', '1', //temp data CE D3 D1 41 (little endian)
            '5', 'F',             //CRC
            0x0D                  // footer
        });

    uint64_t v;
    Context->ReadDirectRegister(0, &v, Float, 4);
    ASSERT_EQ(0x41D1D3CE, v); //big-endian


	// >> 24 30 30 30 31 52 52 30 30 30 34 30 34 62 31 0d
	// << 21 30 30 30 31 52 52 33 30 39 41 45 42 34 31 34 46 0D
    // humidity == 29.450287

    SerialPort->EnqueueResponse(
        {
            // Session setup response
            '!',                  // header
            '0', '0', '0', '1',   // slave addr
            'R', 'R',             // read response
            '3', '0', '9', 'A', 'E', 'B', '4', '1', //hum data 30 9A EB 41 (little endian)
            '4', 'F',             //CRC
            0x0D                  // footer
        });

    Context->ReadDirectRegister(4, &v, Float, 4);
    ASSERT_EQ(0x41EB9A30, v); //big-endian


    Context->EndPollCycle(0);
    Context->Disconnect();
}