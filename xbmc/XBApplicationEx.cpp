/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://kodi.tv
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "FileItem.h"
#include "messaging/ApplicationMessenger.h"
#include "PlayListPlayer.h"
#include "XBApplicationEx.h"
#include "utils/log.h"
#include "threads/SystemClock.h"
#include "commons/Exception.h"
#ifdef TARGET_POSIX
#include "platform/linux/XTimeUtils.h"
#endif
#include "AppParamParser.h"

// Put this here for easy enable and disable
#ifndef _DEBUG
#define XBMC_TRACK_EXCEPTIONS
#endif

CXBApplicationEx::CXBApplicationEx()
{
  // Variables to perform app timing
  m_bStop = false;
  m_AppFocused = true;
  m_ExitCode = EXITCODE_QUIT;
  m_renderGUI = false;
}

CXBApplicationEx::~CXBApplicationEx() = default;

/* Destroy the app */
void CXBApplicationEx::Destroy()
{
  CLog::Log(LOGNOTICE, "destroy");
  // Perform app-specific cleanup
  Cleanup();
}

/* Function that runs the application */
int CXBApplicationEx::Run(const CAppParamParser &params)
{
  CLog::Log(LOGNOTICE, "Running the application..." );

  unsigned int lastFrameTime = 0;
  unsigned int frameTime = 0;
  const unsigned int noRenderFrameTime = 15;  // Simulates ~66fps

  if (params.Playlist().Size() > 0)
  {
    CServiceBroker::GetPlaylistPlayer().Add(0, params.Playlist());
    CServiceBroker::GetPlaylistPlayer().SetCurrentPlaylist(0);
    KODI::MESSAGING::CApplicationMessenger::GetInstance().PostMsg(TMSG_PLAYLISTPLAYER_PLAY, -1);
  }

  // Run xbmc
  while (!m_bStop)
  {
    //-----------------------------------------
    // Animate and render a frame
    //-----------------------------------------
#ifdef XBMC_TRACK_EXCEPTIONS
    try
    {
#endif
      lastFrameTime = XbmcThreads::SystemClockMillis();
      Process(true);
      //reset exception count
#ifdef XBMC_TRACK_EXCEPTIONS

    }
    catch (const XbmcCommons::UncheckedException &e)
    {
      e.LogThrowMessage("CApplication::Process()");
      throw;
    }
    catch (...)
    {
      CLog::Log(LOGERROR, "exception in CApplication::Process()");
      throw;
    }
#endif
    // Frame move the scene
#ifdef XBMC_TRACK_EXCEPTIONS
    try
    {
#endif
      if (!m_bStop)
      {
        FrameMove(true, m_renderGUI);
      }

      //reset exception count
#ifdef XBMC_TRACK_EXCEPTIONS
    }
    catch (const XbmcCommons::UncheckedException &e)
    {
      e.LogThrowMessage("CApplication::FrameMove()");
      throw;
    }
    catch (...)
    {
      CLog::Log(LOGERROR, "exception in CApplication::FrameMove()");
      throw;
    }
#endif

    // Render the scene
#ifdef XBMC_TRACK_EXCEPTIONS
    try
    {
#endif
      if (m_renderGUI && !m_bStop)
      {
        Render();
      }
      else if (!m_renderGUI)
      {
        frameTime = XbmcThreads::SystemClockMillis() - lastFrameTime;
        if(frameTime < noRenderFrameTime)
          Sleep(noRenderFrameTime - frameTime);
      }
#ifdef XBMC_TRACK_EXCEPTIONS
    }
    catch (const XbmcCommons::UncheckedException &e)
    {
      e.LogThrowMessage("CApplication::Render()");
      throw;
    }
    catch (...)
    {
      CLog::Log(LOGERROR, "exception in CApplication::Render()");
      throw;
    }
#endif
  } // while (!m_bStop)
  Destroy();

  CLog::Log(LOGNOTICE, "application stopped..." );
  return m_ExitCode;
}
