#pragma once

namespace BK64 {
enum class ResourceType {
    Animation = 0x414E494D,
    Binary = 0x43504C59,
    DemoInput = 0x454E5653,
    Dialog = 0x4C494D42,
    GruntyQuestion = 0x4D534720,
    LevelSetup = 0x4D534754,
    Midi = 0x534B454C,
    Model = 0x53435250,
    QuizQuestion = 0x53434D44,
    Sprite = 0x48544258,

    // NAudio v0
    Bank = 0x42414E4B,         // BANK
    Sample = 0x41554643,       // AIFC
    Sequence = 0x53455143,     // SEQC

    // NAudio v1
    SoundFont = 0x53464E54,    // SFNT
    Drum = 0x4452554D,         // DRUM
    Instrument = 0x494E5354,   // INST
    AdpcmLoop = 0x4150434C,    // APCL
    AdpcmBook = 0x41504342,    // APCB
    Envelope = 0x45564C50,     // EVLP
    AudioTable = 0x4154424C    // ATBL
};
} // namespace SOH
