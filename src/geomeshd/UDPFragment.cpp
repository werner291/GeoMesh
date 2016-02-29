//
// Created by System Administrator on 2/29/16.
//

#include "UDPFragment.h"

UDPFragment::UDPFragment(uint8_t* buffer, int bufferLength) {

    data.resize(bufferLength);

    memcpy(data.data(), buffer, bufferLength);

}