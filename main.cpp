#include <iostream>
#include "MusicPlayer.h"
using namespace raspberry;

int main()
{
    while(time(nullptr))
    {
        MusicPlayer::Init("play_list.json");

        while (!MusicPlayer::IsDayChanged())
        {
            MusicPlayer::Update();
        }

        std::cout << "The day has been changed." << std::endl;
        MusicPlayer::Shutdown();
    }

    return 0;
}
