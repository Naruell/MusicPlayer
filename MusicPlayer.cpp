#include "MusicPlayer.h"

#include <iostream>
#include <fstream>
#include "json.h"

constexpr int MIN_HOUR = 0;
constexpr int MAX_HOUR = 23;
constexpr int MIN_MIN = 0;
constexpr int MAX_MIN = 59;

namespace raspberry
{
    MusicTrack::MusicTrack(time_t playTime, const std::string& musicPath)
        : mPlayTime(playTime), mMusicPath(musicPath), mMusicPathItr(mMusicPath.begin()), mIsPlaying(false), mIsEmpty(false)
    {
        if(mMusicPath.empty()) { mIsEmpty = true; }
    }

    void MusicTrack::Insert()
    {
        mIsPlaying = true;
    }

    void MusicTrack::Update(sf::Music& music)
    {
        switch(music.getStatus())
        {
            case sf::Music::Status::Stopped:
                if(mMusicPath.empty())
                {
                    mIsPlaying = false;
                    mIsEmpty = true;
                    return;
                }
                else
                {
                    if(music.openFromFile(PopPlayPath()))
                    {
                        music.play();
                    }
                }
                break;

            case sf::Music::Status::Paused:
            case sf::Music::Status::Playing:
                break;
        }
    }

    bool MusicTrack::IsPlaying() const
    {
        return mIsPlaying;
    }

    bool MusicTrack::IsEmpty() const
    {
        return mIsEmpty;
    }

    time_t MusicTrack::GetPlayTime() const
    {
        return mPlayTime;
    }

    const std::string& MusicTrack::GetMusicPath() const
    {
        return mMusicPath;
    }

    std::string MusicTrack::PopPlayPath()
    {
        if( mMusicPathItr == mMusicPath.end() )
        {
            return "error";
        }

        int startIndex = std::distance(mMusicPath.begin(), mMusicPathItr);
        int i = 0;

        for(std::string::iterator& itr = mMusicPathItr; mMusicPathItr != mMusicPath.end(); ++mMusicPathItr)
        {
            if( *itr == '?')
            {
                ++mMusicPathItr;
                break;
            }
            else { ++i; }
        }

        std::string path = mMusicPath.substr(startIndex, i);

        if( mMusicPathItr == mMusicPath.end() )
        {
            mMusicPath.clear();
            mMusicPathItr = mMusicPath.begin();
        }
        return path;
    }

    bool operator<(const std::unique_ptr<MusicTrack>& lhs, const std::unique_ptr<MusicTrack>& rhs)
    {
        return (lhs->GetPlayTime() > rhs->GetPlayTime());
    }

    bool MusicPlayer::Init(const std::string &playListPath)
    {
        std::cout << "MusicPlayer::Init() called." << std::endl;

        // Member variable assign
        if(mPreTimer == -1) { mPreTimer = time(nullptr); }
        mCurTimer = time(nullptr);

        // variables for json parsing
        Json::CharReaderBuilder builder;
        builder["collectComments"] = false;
        Json::Value value;
        JSONCPP_STRING err;

        // temp variables
        time_t curTime_t = time(nullptr);
        tm* currentTime = localtime(&curTime_t);
        tm trackPlayTime;
        std::string tempStr;
        int hour, min;
        int strBegin = 0;

        std::ifstream json_file;

        // if playListPath is not JSON extension
        if(playListPath.substr(playListPath.find_last_of('.')) != ".json" &&
           playListPath.substr(playListPath.find_last_of('.')) != ".JSON")
        {
            std::cout << "MusicPlayer::Init() : [" << playListPath << "] is not a JSON file" << std::endl;
            return false;
        }

        json_file.open(playListPath);

        bool isOpened = Json::parseFromStream(builder, json_file, &value, &err);
        if(!isOpened)
        {
            std::cout << "MusicPlayer::Init() : Failed to open [" << playListPath << "]."<< std::endl;
            return false;
        }
        else
        {
            for(const auto& playList : value["playList"])
            {
                tempStr = playList["playTime"].asString();
                strBegin = 0;

                hour = atoi( tempStr.substr(strBegin).substr(0, tempStr.find_first_of(':') - strBegin).c_str());
                strBegin = tempStr.substr(strBegin).find_first_of(':') + 1;

                min = atoi( tempStr.substr(strBegin).substr(0, tempStr.find_first_of(':') - strBegin).c_str());
                strBegin = tempStr.substr(strBegin).find_first_of(':') + 1;

                // If you want to parse a "second", repeat above
                // sec = atoi( tempStr.substr(strBegin).substr(0, tempStr.find_first_of(':') - strBegin).c_str());
                // strBegin = tempStr.substr(strBegin).find_first_of(':') + 1;

                if( MIN_HOUR > hour || hour > MAX_HOUR )
                {
                    std::cout << "MusicPlayer::Init() : Invalid hour range [" << hour << "]. Track removed." << std::endl;
                    continue;
                }
                if(MIN_MIN > min || min > MAX_MIN)
                {
                    std::cout << "MusicPlayer::Init() : Invalid minute range [" << min << "]. Track removed." << std::endl;
                    continue;
                }

                trackPlayTime.tm_year = currentTime->tm_year;
                trackPlayTime.tm_mon = currentTime->tm_mon;
                trackPlayTime.tm_mday = currentTime->tm_mday;
                trackPlayTime.tm_hour = hour;
                trackPlayTime.tm_min = min;
                trackPlayTime.tm_sec = 0;
                trackPlayTime.tm_isdst = 0;

                tempStr = "";
                for(const auto& track : playList["track"])
                {
                    tempStr += track.asString();
                    tempStr += "?";
                }

                mTrackQueue.emplace( new MusicTrack(mktime(&trackPlayTime), tempStr) );
                std::cout << "MusicPlayer::Init() : Track [" << hour << ":" << min <<"] loaded." << std::endl;
            }
        }
        std::cout << "MusicPlayer::Init() ended." << std::endl;
        return true;
    }

    void MusicPlayer::Shutdown()
    {
        std::cout << "MusicPlayer::Shutdown() called." << std::endl;

        while(!mTrackQueue.empty())
        {
            mTrackQueue.pop();
        }
        std::cout << "MusicPlayer::Shutdown() ended." << std::endl;
    }

    void MusicPlayer::Update()
    {
        mPreTimer = mCurTimer;
        mCurTimer = time(nullptr);

        // Check if the track in top of the queue is empty or passed
        if(!mTrackQueue.empty())
        {
            if(mTrackQueue.top()->IsEmpty())
            {
                std::cout << "MusicPlayer::Update() : An empty track has been removed!" << std::endl;
                mTrackQueue.pop();
            }
            else if( !mTrackQueue.top()->IsPlaying() && mPreTimer >= mTrackQueue.top()->GetPlayTime())
            {
                std::cout << "MusicPlayer::Update() : A passed track has been removed!" << std::endl;
                mTrackQueue.pop();
            }
        }

        if(!mTrackQueue.empty())
        {
            if(mPreTimer <  mTrackQueue.top()->GetPlayTime() &&
               mCurTimer >= mTrackQueue.top()->GetPlayTime() )
            {
                std::cout << "MusicPlayer::Update() : Reached the time, play the track!" << std::endl;
                PlayTrack();
            }

            if(mTrackQueue.top()->IsPlaying())
            {
                mTrackQueue.top()->Update(music);
            }
        }
    }

    bool MusicPlayer::IsEmpty()
    {
        return mTrackQueue.empty();
    }

    bool MusicPlayer::IsDayChanged()
    {
        int preTimer = localtime(&mPreTimer)->tm_mday;
        int curTimer = localtime(&mCurTimer)->tm_mday;
        return (preTimer != curTimer);
    }

    void MusicPlayer::PlayTrack()
    {
        if(mTrackQueue.empty())
        {
            return;
        }

        if(mTrackQueue.top()->IsPlaying()) // When track is on playing
        {
        }
        else if(mTrackQueue.top()->IsEmpty()) // When track is empty (track ended)
        {
            mTrackQueue.pop();
            PlayTrack();
        }
        else // When track is playable
        {
            mTrackQueue.top()->Insert();
        }
    }

    void MusicPlayer::SkipMusic()
    {
        music.stop();
    }

    void MusicPlayer::SkipTrack()
    {
        SkipMusic();
        mTrackQueue.pop();
    }

    void MusicPlayer::PrintTrackInfo()
    {
        std::cout << "Track Count : " << mTrackQueue.size() << std::endl;
        while(!mTrackQueue.empty())
        {
            const std::unique_ptr<MusicTrack>& track = mTrackQueue.top();
            std::cout << "Track for " << track->GetPlayTime() << std::endl;

            for(const auto& c : mTrackQueue.top()->GetMusicPath())
            {
                if( c == '?') { std::cout << std::endl; }
                else { std::cout << c; }
            }
            mTrackQueue.pop();
        }
    }
}