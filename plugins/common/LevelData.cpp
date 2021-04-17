#include "LevelData.hpp"
#include <iostream>

LevelData::LevelData() : m_sharedMemory("rtjamValues") {
    m_sharedMemory.Create(sizeof(RTJamLevels));
    m_sharedMemory.Attach();
    m_pJamLevels = (RTJamLevels*) m_sharedMemory.GetData();
}

LevelData::~LevelData() {
    m_sharedMemory.Detach();
}

void LevelData::lock() {
    m_sharedMemory.Lock();
}

void LevelData::unlock() {
    m_sharedMemory.UnLock();
}