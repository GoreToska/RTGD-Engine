//
// Created by ivan on 9/24/26.
//

#pragma once

#include "AssetLoader/AssetManager.h"
#include "AssetLoader/PathResolve.h"
#include "AssetLoader/Refs/AssetRef.h"
#include "Systems/AudioSystem.h"
#include "Tools/Alias.h"

namespace RTGDEngine
{
    struct AudioSourceComponent
    {
        BankRef Bank;
        std::string Event;
        bool PlayOnStart = true;

        // transient
        AudioEvent Playing;
        bool Started = false;

        void Play(const Float3& position)
        {
            Playing.Stop();
            Playing = GAudio().Play(Event, position);
        }

        void Stop(EStopMode mode = EStopMode::Immediate)
        {
            Playing.Stop(mode);
        }

        void SetParameter(const char* name, float v)
        {
            Playing.SetParameter(name, v);
        }

        bool IsPlaying() const
        {
            return Playing.IsPlaying();
        }

        static void RegisterMeta(const World& world)
        {
            world.component<AudioSourceComponent>("AudioSourceComponent")
                    .member<BankRef>("Bank")
                    .member<std::string>("Event")
                    .member<bool>("PlayOnStart");

            world.observer<AudioSourceComponent>().event(flecs::OnSet)
                    .each([](AudioSourceComponent& comp)
                    {
                        if (!comp.Bank.Path.empty())
                            comp.Bank.Resolve(GAssets().GetBank(GetAbsolutePath(comp.Bank.Path)));
                    });

            world.observer<AudioSourceComponent>().event(flecs::OnRemove)
                    .each([](AudioSourceComponent& comp)
                    {
                        comp.Playing.Stop();
                    });
        }
    };

    struct AudioListenerComponent
    {
        static void RegisterMeta(const World& world)
        {
            world.component<AudioListenerComponent>("AudioListenerComponent");
        }
    };
}
