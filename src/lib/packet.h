#pragma once
#include<stdint.h>
#include<assert.h>
#include<Arduino.h>
using namespace std;

/*
Packet Architecture
+---+----+--------+-----+----+---+--------+---+
|STX|DATATYPE|D_LEN|DATA|...|CHECKSUM|ETX|  ==> 일반적인 패킷 형태
+---+----+--------+-----+----+---+--------+---+
*/

// STX, ETX
#define STX 0x06
#define ETX 0x08

//DATATYPE
#define SENSOR 0
#define CMD 1
#define STRING 2
#define BMP 3
#define FILE_DATA 4
#define ROCKETSTATE 5
#define NULLPACKET 6
#define ACK 7

//MAX PACKET SIZE: 사용자 정의
#define MAX_PACKET_SIZE 100000

class Packet {
public:
    //위성에서 지상국으로 데이터 전송 시, 사용하는 패킷 객체
    Packet(uint8_t dtype, uint8_t D_LEN, uint8_t* data) {
        _dtype = dtype;
        _D_LEN = D_LEN;

        _copyData(data);

        _getPacketSize();
        _packet = new uint8_t[_PACKETSIZE];

        _packet[0] = STX;
        _packet[1] = _dtype;
        _packet[2] = _D_LEN;

        int32_t idx = 3;
        for (int i = 0; i < _D_LEN; i++) {
            _packet[idx] = _data[i];
            idx++;
        }

        _getCheckSum();

        _packet[idx++] = _checksum;
        _packet[idx] = ETX;

    }

    //위성에서 받은 패킷 데이터를 바탕으로 지상국에서 객체 생성 과정
    Packet(uint8_t* Packet, int32_t PacketSize) {
        _PACKETSIZE = PacketSize;
        _copyPacket(Packet);
        _dtype = _packet[1];
        _getDataSize();

        _recived_checkSum = _packet[_PACKETSIZE - 2];
        _getCheckSum();
    }

    Packet(){
        _dtype = NULLPACKET;
        _D_LEN = 0;

        _getPacketSize();
        _packet = new uint8_t[_PACKETSIZE];

        _packet[0] = uint8_t(STX);
        _packet[1] = _dtype;
        _packet[2] = _D_LEN;

        int32_t idx = 3;
        _getCheckSum();

        _packet[idx++] = _checksum;
        _packet[idx] = uint8_t(ETX);
    }

    ~Packet() {
        delete[] _packet;
        delete[] _data;
    }

    uint8_t getPacketData(int idx) {
        Serial.println(_packet[idx]);
        return _packet[idx];
    }

    uint8_t* getData() {
        return _data;
    }

    uint32_t getPacketSize() {
        return _PACKETSIZE;
    }

    int getDataSize() {
        return _D_LEN;
    }

    uint8_t getDataType(){
        return _packet[2];
    }


    Packet& operator=(const Packet& obj) {
        this->_dtype = obj._dtype;
        this->_D_LEN = obj._D_LEN;
        this->_checksum = obj._checksum;
        this->_PACKETSIZE = obj._PACKETSIZE;
        this->_copyData(obj._data);

        this->_packet = new uint8_t[_PACKETSIZE];
        for (int i = 0; i < this->_PACKETSIZE; i++) {
            this->_packet[i] = obj._packet[i];
        }

        return *this;
    }

    template<class T>
    float dataToNum(uint8_t start) {
        float result;
        memcpy(&result, &_data[start], sizeof(float));
        
        return result;
    }

    void transmitPacket(){
        float dataArray[9];
        for(int i=0;i<9;i++){
            dataArray[i] = dataToNum<float>(i*4);
            Serial.print(dataArray[i]);
            Serial.print(" ");
        }
        Serial.println();
        Serial.print("---------------------------------------------------------\n");
        for(int i=0;i<_PACKETSIZE;i++){
            Serial.print(_packet[i]);
            Serial.print(" ");
            Serial3.write(_packet[i]);
        }
        Serial.println();
        //Serial3.write(_packet, size_t(_PACKETSIZE));
        Serial.print("---------------------------------------------------------\n");
    }

private:
    uint8_t* _packet;
    uint8_t* _data;
    uint8_t _dtype;
    uint8_t _D_LEN;
    uint8_t _checksum, _recived_checkSum;

    // 데이터 사이즈는 최대 4바이트 정수 범위
    int _PACKETSIZE;

    inline void _copyData(uint8_t* data) {
        _data = new uint8_t[_D_LEN];
        for (int i = 0; i < _D_LEN; i++) {
            _data[i] = data[i];
        }
    }

    inline void _copyPacket(uint8_t* packet) {
        _packet = new uint8_t[_PACKETSIZE];
        for (int i = 0; i < _PACKETSIZE; i++) {
            _packet[i] = packet[i];
        }
    }

    inline void _getPacketSize() {
        //packet의 데이터 종류 5개 + 데이터 길이
        _PACKETSIZE = 5 + _D_LEN;
    }

    inline void _getDataSize() {
        _D_LEN = _PACKETSIZE - 5;
    }

    inline void _getCheckSum() {
        for (int i = 0; i < _PACKETSIZE - 2; i++) {
            _checksum ^= _packet[i];//XOR 연산으로 parity 검사
        }
        _checksum ^= ETX;
    }
};
