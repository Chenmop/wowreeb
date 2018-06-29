/*
    MIT License

    Copyright (c) 2018 namreeb http://github.com/namreeb legal@namreeb.org

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

*/

#pragma comment(lib, "asmjit.lib")
#pragma comment(lib, "udis86.lib")

#include "InitializeHooks.hpp"

#include <hadesmem/patcher.hpp>

#include <cstdint>

namespace
{
enum class Version
{
    Classic = 0,
    TBC,
    WotLK,
    Cata32,
    Cata64,
    Total
};

enum class Offset
{
    CVar__Set = 0,
    RealmListCVar,
    Initialize,
    Login,
    FoV,
    Total
};

PVOID GetAddress(Version version, Offset offset)
{
    static constexpr std::uint32_t offsets[static_cast<int>(Version::Total)][static_cast<int>(Offset::Total)] =
    {
        // Classic
        {
            0x23DF50,
            0x82812C,
            0x2AD0,
            0x6AFB0,
            0x4089B4
        },
        // TBC
        {
            0x23F6C0,
            0x943330,
            0x77AC0,
            0x6E560,
            0x4B5A04
        },
        // WotLK
        {
            0x3668C0,
            0x879D00,
            0xE5940,
            0xD8A30,
            0x5E8D88
        },
        // Cata32
        {
            0x2553B0,
            0x9BE800,
            0x40F8F0,
            0x400240,
            0x00, // not supported.  let me know if anyone actually wants this
        },
        // Cata64
        {
            0x2F61D0,
            0xCA4328,
            0x51C0F0,
            0x514100,
            0x00, // not supported.  let me know if anyone actually wants this
        }
    };

    auto const baseAddress = reinterpret_cast<std::uint8_t *>(::GetModuleHandle(nullptr));

    return baseAddress + offsets[static_cast<int>(version)][static_cast<int>(offset)];
}
}

namespace Classic
{
class CVar {};

using SetT = bool(__thiscall CVar::*)(const char *, char, char, char, char);
using InitializeT = void (__cdecl *)();
using LoginT = void(__fastcall *)(char *, char *);

void InitializeHook(hadesmem::PatchDetourBase *detour, GameSettings *settings)
{
    auto const initialize = detour->GetTrampolineT<InitializeT>();
    initialize();

    auto const cvar = *reinterpret_cast<CVar **>(GetAddress(Version::Classic, Offset::RealmListCVar));
    auto const set = hadesmem::detail::AliasCast<SetT>(GetAddress(Version::Classic, Offset::CVar__Set));

    (cvar->*set)(settings->AuthServer, 1, 0, 1, 0);

    detour->Remove();

    if (settings->CredentialsSet)
    {
        auto const login = hadesmem::detail::AliasCast<LoginT>(GetAddress(Version::Classic, Offset::Login));
        login(settings->Username, settings->Password);
    }

    settings->LoadComplete = true;
}

void ApplyClientInitHook(GameSettings *settings)
{
    auto const proc = hadesmem::Process(::GetCurrentProcessId());
    auto const initializeOrig = hadesmem::detail::AliasCast<InitializeT>(GetAddress(Version::Classic, Offset::Initialize));
    auto initializeDetour = new hadesmem::PatchDetour<InitializeT>(proc, initializeOrig,
        [settings] (hadesmem::PatchDetourBase *detour)
        {
            InitializeHook(detour, settings);
        });

    initializeDetour->Apply();

    if (settings->FoVSet)
    {
        auto const pFov = GetAddress(Version::Classic, Offset::FoV);
        std::vector<std::uint8_t> patchData(sizeof(settings->FoV));
        memcpy(&patchData[0], &settings->FoV, sizeof(settings->FoV));
        auto patch = new hadesmem::PatchRaw(proc, pFov, patchData);
        patch->Apply();
    }
}
}

namespace TBC
{
class CVar {};

using SetT = bool(__thiscall CVar::*)(const char *, char, char, char, char);
using InitializeT = void(__cdecl*)();
using LoginT = void(__cdecl *)(char *, char *);

void InitializeHook(hadesmem::PatchDetourBase *detour, GameSettings *settings)
{
    auto const initialize = detour->GetTrampolineT<InitializeT>();
    initialize();

    auto const cvar = *reinterpret_cast<CVar **>(GetAddress(Version::TBC, Offset::RealmListCVar));
    auto const set = hadesmem::detail::AliasCast<SetT>(GetAddress(Version::TBC, Offset::CVar__Set));

    (cvar->*set)(settings->AuthServer, 1, 0, 1, 0);

    detour->Remove();

    if (settings->CredentialsSet)
    {
        auto const login = hadesmem::detail::AliasCast<LoginT>(GetAddress(Version::TBC, Offset::Login));
        login(settings->Username, settings->Password);
    }

    settings->LoadComplete = true;
}

void ApplyClientInitHook(GameSettings *settings)
{
    auto const proc = hadesmem::Process(::GetCurrentProcessId());
    auto const initializeOrig = hadesmem::detail::AliasCast<InitializeT>(GetAddress(Version::TBC, Offset::Initialize));
    auto initializeDetour = new hadesmem::PatchDetour<InitializeT>(proc, initializeOrig,
        [settings] (hadesmem::PatchDetourBase *detour)
        {
            InitializeHook(detour, settings);
        });

    initializeDetour->Apply();

    if (settings->FoVSet)
    {
        auto const pFov = GetAddress(Version::TBC, Offset::FoV);
        std::vector<std::uint8_t> patchData(sizeof(settings->FoV));
        memcpy(&patchData[0], &settings->FoV, sizeof(settings->FoV));
        auto patch = new hadesmem::PatchRaw(proc, pFov, patchData);
        patch->Apply();
    }
}
}

namespace WOTLK
{
class CVar {};

using SetT = bool(__thiscall CVar::*)(const char *, char, char, char, char);
using InitializeT = void (*)();
using LoginT = void(__cdecl *)(char *, char *);

void InitializeHook(hadesmem::PatchDetourBase *detour, GameSettings *settings)
{
    auto const initialize = detour->GetTrampolineT<InitializeT>();
    initialize();

    auto const cvar = *reinterpret_cast<CVar **>(GetAddress(Version::WotLK, Offset::RealmListCVar));
    auto const set = hadesmem::detail::AliasCast<SetT>(GetAddress(Version::WotLK, Offset::CVar__Set));

    (cvar->*set)(settings->AuthServer, 1, 0, 1, 0);

    detour->Remove();

    if (settings->CredentialsSet)
    {
        auto const login = hadesmem::detail::AliasCast<LoginT>(GetAddress(Version::WotLK, Offset::Login));
        login(settings->Username, settings->Password);
    }

    settings->LoadComplete = true;
}

void ApplyClientInitHook(GameSettings *settings)
{
    auto const proc = hadesmem::Process(::GetCurrentProcessId());
    auto const initializeOrig = hadesmem::detail::AliasCast<InitializeT>(GetAddress(Version::WotLK, Offset::Initialize));
    auto initializeDetour = new hadesmem::PatchDetour<InitializeT>(proc, initializeOrig,
        [settings] (hadesmem::PatchDetourBase *detour)
        {
            InitializeHook(detour, settings);
        });

    initializeDetour->Apply();

    if (settings->FoVSet)
    {
        auto const pFov = GetAddress(Version::WotLK, Offset::FoV);
        std::vector<std::uint8_t> patchData(sizeof(settings->FoV));
        memcpy(&patchData[0], &settings->FoV, sizeof(settings->FoV));
        auto patch = new hadesmem::PatchRaw(proc, pFov, patchData);
        patch->Apply();
    }
}
}

namespace Cata32
{
class CVar {};

using SetT = bool(__thiscall CVar::*)(const char *, char, char, char, char);
using InitializeT = void(__cdecl *)();
using LoginT = void(__cdecl *)(char *, char *);

void InitializeHook(hadesmem::PatchDetourBase *detour, GameSettings *settings)
{
    auto const initialize = detour->GetTrampolineT<InitializeT>();
    initialize();

    auto const cvar = *reinterpret_cast<CVar **>(GetAddress(Version::Cata32, Offset::RealmListCVar));
    auto const set = hadesmem::detail::AliasCast<SetT>(GetAddress(Version::Cata32, Offset::CVar__Set));

    (cvar->*set)(settings->AuthServer, 1, 0, 1, 0);

    detour->Remove();

    if (settings->CredentialsSet)
    {
        auto const login = hadesmem::detail::AliasCast<LoginT>(GetAddress(Version::Cata32, Offset::Login));
        login(settings->Username, settings->Password);
    }

    settings->LoadComplete = true;
}

void ApplyClientInitHook(GameSettings *settings)
{
    auto const proc = hadesmem::Process(::GetCurrentProcessId());
    auto const initializeOrig = hadesmem::detail::AliasCast<InitializeT>(GetAddress(Version::Cata32, Offset::Initialize));
    auto initializeDetour = new hadesmem::PatchDetour<InitializeT>(proc, initializeOrig,
        [settings](hadesmem::PatchDetourBase *detour)
        {
            InitializeHook(detour, settings);
        });

    initializeDetour->Apply();
}
}

namespace Cata64
{
class CVar {};

using SetT = bool(__fastcall CVar::*)(const char *, char, char, char, char);
using InitializeT = int (__stdcall *)();
using LoginT = void(__fastcall *)(char *, char *);

int InitializeHook(hadesmem::PatchDetourBase *detour, GameSettings *settings)
{
    auto const initialize = detour->GetTrampolineT<InitializeT>();
    auto const ret = initialize();

    auto const cvar = *reinterpret_cast<CVar **>(GetAddress(Version::Cata64, Offset::RealmListCVar));
    auto const set = hadesmem::detail::AliasCast<SetT>(GetAddress(Version::Cata64, Offset::CVar__Set));

    (cvar->*set)(settings->AuthServer, 1, 0, 1, 0);

    detour->Remove();

    if (settings->CredentialsSet)
    {
        auto const login = hadesmem::detail::AliasCast<LoginT>(GetAddress(Version::Cata64, Offset::Login));
        login(settings->Username, settings->Password);
    }

    settings->LoadComplete = true;

    return ret;
}

void ApplyClientInitHook(GameSettings *settings)
{
    auto const proc = hadesmem::Process(::GetCurrentProcessId());
    auto const initializeOrig = hadesmem::detail::AliasCast<InitializeT>(GetAddress(Version::Cata64, Offset::Initialize));
    auto initializeDetour = new hadesmem::PatchDetour<InitializeT>(proc, initializeOrig,
        [settings] (hadesmem::PatchDetourBase *detour)
        {
            return InitializeHook(detour, settings);
        });

    initializeDetour->Apply();
}
}