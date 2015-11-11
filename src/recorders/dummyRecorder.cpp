#include "recorders/dummyRecorder.hpp"
#include "G4Run.hh"
#include "G4Event.hh"

DummyRecorder::DummyRecorder() : RecorderBase() {}

DummyRecorder::~DummyRecorder() {}

void DummyRecorder::recordBeginOfRun(const G4Run* /*run*/) {}

void DummyRecorder::recordEndOfRun(const G4Run* /*run*/) {}

