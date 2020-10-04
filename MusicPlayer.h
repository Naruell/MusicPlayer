#pragma once
#include <ctime>
#include <string>
#include <memory>
#include <queue>

#include <SFML/Audio.hpp>

namespace raspberry
{
    class MusicTrack
    {
    public:
        MusicTrack(time_t playTime, const std::string& musicPath);

        void Insert();
        void Update(sf::Music& music);

        // Getter
        bool IsPlaying() const;
        bool IsEmpty() const;

        time_t GetPlayTime() const;
        const std::string& GetMusicPath() const;

    private:
        std::string PopPlayPath();

        time_t mPlayTime;
        std::string mMusicPath;
        std::string::iterator mMusicPathItr;

        bool mIsPlaying;
        bool mIsEmpty;
    };

    bool operator<(const std::unique_ptr<MusicTrack> &lhs, const std::unique_ptr<MusicTrack> &rhs);

    class MusicPlayer
    {
    public:
        MusicPlayer() = delete;

        static bool Init(const std::string& playListPath);
        static void Shutdown();

        static void Update();
        static bool IsEmpty();
        static bool IsDayChanged();

        static void PlayTrack();
        static void SkipMusic();
        static void SkipTrack();

        static void PrintTrackInfo();

    private:
        inline static time_t mPreTimer = -1;
        inline static time_t mCurTimer = -1;

        inline static std::priority_queue<std::unique_ptr<MusicTrack>> mTrackQueue;

        inline static sf::Music music;
    };
}