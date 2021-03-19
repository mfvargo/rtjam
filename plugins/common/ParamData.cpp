#include "ParamData.hpp"

ParamData::ParamData() : m_queue("/rtjamParams", sizeof(RTJamParam)) {
}

ParamData::~ParamData() {
}

void ParamData::flush() {
  m_queue.flush();
}

void ParamData::send(RTJamParam* param) {
  m_queue.send(param, sizeof(RTJamParam));
}

void ParamData::receive(RTJamParam* param) {
  m_queue.recv(param, sizeof(RTJamParam));
}