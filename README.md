# lameo

Uhh, I don't know. I want it to at least run on both Windows and Linux, though.

This is a solo project. You can look at it, otherwise suggest things through
issues and/or pull requests, even if you're a Schwungus dev.

## External Dependencies

### [FMOD Engine 2.03.08](https://www.fmod.com/download#fmodengine)

Move everything from `C:\Program Files (x86)\FMOD SoundSystem\FMOD Studio API Windows\api\core`
to `include/fmod/windows` on Windows. The process on Linux is similar.

### [Steamworks SDK 1.62](https://partner.steamgames.com/dashboard)

Place the SDK's zip and a `steam_appid.txt` with the application ID of your
choice in `include/steam`.

## Legalese

lameo is licensed under the [MIT license](https://github.com/Schwungus/lameo/blob/main/LICENSE).

-   Simple DirectMedia Layer © 1997-2025 Sam Lantinga ([Zlib license](https://github.com/libsdl-org/SDL/blob/main/LICENSE.txt))
-   glad © 2013-2022 David Herberth ([MIT license](https://github.com/Dav1dde/glad/blob/glad2/LICENSE))
-   cglm © 2015 Recep Aslantas ([MIT license](https://github.com/recp/cglm/blob/master/LICENSE))
-   FMOD Engine © 1995-2025 Firelight Technologies Pty Ltd. ([FMOD licensing](https://fmod.com/licensing))
-   yyjson © 2020 Yaoyuan Guo ([MIT license](https://github.com/ibireme/yyjson/blob/master/LICENSE))
-   Lua © 1994-2025 Lua.org, PUC-Rio ([MIT license](https://www.lua.org/license.html))
-   Steamworks © 2008-2025 Valve Corporation ([Steamworks SDK license](https://partner.steamgames.com/documentation/sdk_access_agreement))
-   BBMOD © 2020-2024 BlueBurn ([MIT license](https://github.com/blueburncz/BBMOD/blob/bbmod3/LICENSE))
