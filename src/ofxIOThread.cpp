
// This thread class was take from the excelent addon ofxIO written by Christopher Baker.
// http://bakercp.github.io/ofxIO/
// I only need this class from it, so I just provide it here to avoid unnecesary stuff
//
// Copyright (c) 2017 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier:	MIT
//


#include "ofxIOThread.h"
#include "ofLog.h"



ofxIOThread::ofxIOThread(std::function<void()> threadedFunction):
    _exitListener(ofEvents().exit.newListener(this, &ofxIOThread::_exit)),
    _updateListener(ofEvents().update.newListener(this, &ofxIOThread::_update)),
    _threadedFunction(threadedFunction),
    _isRunning(false)
{
}


ofxIOThread::~ofxIOThread()
{
    stopAndJoin();
}


void ofxIOThread::stopAndJoin()
{
    stop();

    try
    {
        _thread.join();
        onThreadJoined();
    }
    catch (...)
    {
        ofLogVerbose("ofxIOThread::stopAndJoin") << "Thread join failed.";
    }
}


void ofxIOThread::stop()
{
    if (_isRunning)
    {
        _isRunning = false;
        condition.notify_all();
        onStopRequested();
    }
}


void ofxIOThread::start()
{
    stopAndJoin();

    if (!_isRunning)
    {
        _thread = std::thread(&ofxIOThread::_run, this);
    }
}


bool ofxIOThread::isRunning() const
{
    return _isRunning;
}


void ofxIOThread::onUpdate()
{
}


void ofxIOThread::onExit()
{
}


void ofxIOThread::onThreadStarted()
{
    ofLogVerbose("ofxIOThread::onThreadStarted") << "On thread started.";
}


void ofxIOThread::onStopRequested()
{
    ofLogVerbose("ofxIOThread::onStopRequested") << "On stop requested.";
}


void ofxIOThread::onThreadFinished()
{
    ofLogVerbose("ofxIOThread::onThreadFinished") << "On thread finished.";
}


void ofxIOThread::onThreadJoined()
{
    ofLogVerbose("ofxIOThread::onThreadJoined") << "On thread joined.";
}


bool ofxIOThread::shouldRepeatWithDelay(uint64_t& delay)
{
    return false;
}


void ofxIOThread::_update(ofEventArgs& evt)
{
    onUpdate();
}


void ofxIOThread::_exit(ofEventArgs& evt)
{
    onExit();
    stopAndJoin();
}


void ofxIOThread::_run()
{
    _isRunning = true;

    onThreadStarted();

    bool repeat = true;
    uint64_t delay = 0;

    do
    {
        try
        {
            if (_threadedFunction)
            {
                _threadedFunction();
            }
            else throw std::runtime_error("The _threadedFunction is a nullptr");
        }
        catch (const std::exception& exc)
        {
            ofLogError("ofxIOThread::_run") << "Exception: " << exc.what();

        }
        catch (...)
        {
            ofLogError("ofxIOThread::_run") << "Unknown exception caught.";
        }

        repeat = shouldRepeatWithDelay(delay);

        if (repeat)
        {
            std::unique_lock<std::mutex> lock(mutex);
            condition.wait_for(lock, std::chrono::milliseconds(delay));
        }
    } while (_isRunning && repeat);

    _isRunning = false;

    onThreadFinished();
}
